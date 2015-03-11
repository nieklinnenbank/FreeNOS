/*
 * Copyright (C) 2009 Niek Linnenbank
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DEVICE_SERVER_H
#define __DEVICE_SERVER_H

#include <API/VMCopy.h>
#include <API/ProcessCtl.h>
#include <FileSystemMessage.h>
#include <FileSystemPath.h>
#include <FileDescriptor.h>
#include <FileType.h>
#include <FileMode.h>
#include <Array.h>
#include <Shared.h>
#include "IPCServer.h"
#include "Device.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

/** Maximum number of devices handled simultaneously by a DeviceServer. */
#define DEVICE_MAX 32

/**
 * @brief Device driver server.
 *
 * Implements IPC handlers to communicate with the FileSystem using
 * FileSystemMessages and invokes Device functions appropriately.
 *
 * @see Device
 * @see FileSystem
 * @see FileSystemMessage
 */
class DeviceServer : public IPCServer<DeviceServer, FileSystemMessage>
{
    public:

	/**
	 * @brief Constructor function.
	 * 
	 * Responsible for registering IPC handlers.
	 *
	 * @param prefix Used to to form the filename of the device files.
	 * @param type FileType of the device files to create.
	 * @param mode Access permissions on the device files.
	 */    
	DeviceServer(const char *prefix, FileType type, FileMode mode = OwnerRW)
	    : IPCServer<DeviceServer, FileSystemMessage>(this), devices(DEVICE_MAX)
	{
	    /* Initialize local member variables. */
	    this->prefix = prefix;
	    this->type   = type;
	    this->mode   = mode;
	    this->files  = new Array<Shared<FileDescriptor> >(MAX_PROCS);
	
	    /* Register IPC Handlers. */
	    addIPCHandler(ReadFile,  &DeviceServer::ioHandler, false);
	    addIPCHandler(WriteFile, &DeviceServer::ioHandler, false);
	    addIPCHandler(SeekFile,  &DeviceServer::ioHandler, false);
	    addIPCHandler(CloseFile, &DeviceServer::ioHandler, false);
	}

	/**
	 * @brief Add a Device.
	 *
	 * Adds an Device to the internal Array of Devices, to
	 * be able to process requests for the Device later on.
	 *
	 * @param dev New device to serve requests for.
	 *
	 * @see Device
	 * @see devices
	 */
	void add(Device *dev)
	{
	    /* Add to the list of Devices. */
	    devices.insert(dev);
	}

	/**
	 * @brief Run the DeviceServer in the background.
	 *
	 * First fork() is used to spawn a child process. The child is
	 * responsible for invoking init() on each Device to run device
	 * specific initialization procedures. Additionally, the child
	 * will ask the kernel to receive InterruptMessages for the registered
	 * interrupt vectors using ProcessCtl. The parent will create a
	 * special device file for each Device in /dev using the prefix
	 * value as a base.
	 *
	 * @return Error code describing how we terminated.
	 *
	 * @see fork
	 * @see mknod
	 * @see mknod
	 * @see prefix
	 * @see ProcessCtl
	 */
	Error run()
	{
	    char path[PATHLEN];
	    pid_t pid;
	    dev_t id;

	    /* If we don't have any Devices, bail out. */
	    for (Size i = 0; i <= devices.size(); i++)
	    {
		if (devices[i])
		{
		    break;
		}
		if (i == devices.size())
		{
		    return EXIT_FAILURE;
		}
	    }
	    
	    /*
	     * Copy ourselves using a fork().
	     */
	    if (!(pid = fork()))
	    {
		/* Register interrupt handlers. */
		for (Size i = 0; i < interrupts.size(); i++)
		{
		    if (interrupts[i])
		    {
		    	/* Register to kernel. */
			ProcessCtl(SELF, WatchIRQ, i);
	    
			/* Register interrupt handler. */
			addIRQHandler(i, &DeviceServer::interruptHandler);
		    }		
		}
		/* Initialize all our Devices. */
		for (Size i = 0; i < devices.size(); i++)
		{
		    if (devices[i])
		    {
			devices[i]->initialize();
		    }
		}
		/* Start processing requests. */
		return IPCServer<DeviceServer, FileSystemMessage>::run();
	    }
	    /*
	     * Loop all registered Devices.
	     */
	    for (Size i = 0; i < devices.size(); i++)
	    {
		/* Skip empty. */
		if (!devices[i])
		{
		    continue;
		}
		/* Attempt to create the device file. */
		for (Size i = 0; i < 1000; i++)
		{
		    /* For a path using the supplied prefix. */		    
		    snprintf(path, sizeof(path), "/dev/%s%u",
			     prefix, i);

		    /*
		     * Use our ProcessID to redirect FileSystemMessages to us.
		     */
		    id.major = pid;
		    id.minor = i;
		
		    /* Create the special device file. */
		    if (mknod(path, (type << FILEMODE_BITS) | mode, id) == 0)
		    {
			break;
		    }
		}
	    }
	    /* All done! */
	    return EXIT_SUCCESS;
	}
	
	/**
	 * @brief Register an interrupt vector for the given device.
	 *
	 * Appends the given Device on the internal list used
	 * for interrupt processing in interruptHandler().
	 *
	 * @param dev Pointer to the Device to wait interrupts for.
	 * @param vector Vector number of the interrupt.
	 *
	 * @see Device
	 * @see interrupts
	 * @see interruptHandler
	 */
	void interrupt(Device *dev, Size vector)
	{
	    if (!interrupts[vector])
	    {
		interrupts.insert(vector, new List<Device>);
	    }
	    interrupts[vector]->insertTail(dev);
	}

    private:

	/**
	 * @brief Input/Output request handler.
	 */    
	void ioHandler(FileSystemMessage *msg)
	{
	    FileDescriptor *fd = getFileDescriptor(files, msg->from, msg->fd);
	    Device *dev;
	    bool result = false;

            /* Do they have this FileDescriptor? */                                
            if (!fd)
	    {
		msg->result = EBADF;
		msg->ipc(msg->from, Send, sizeof(*msg));
		return;
	    }
	    /* Read out values from the FileDescriptor. */
	    else
	    {
		dev = devices[fd->identifier];
		msg->deviceID.minor = fd->identifier;
		
		if (msg->action != SeekFile)
		    msg->offset = fd->position;
	    }
	    
	    /* Do we serve this Device? */
	    if (dev)
	    {
		switch (msg->action)
		{
		    case ReadFile:
			result = performRead(msg);
			break;
		
		    case WriteFile:
			result = performWrite(msg);
			break;
			
		    case SeekFile:
			fd->position = msg->offset;
			msg->result  = ESUCCESS;
			msg->ipc(msg->from, Send, sizeof(*msg));
			return;
		
		    case CloseFile:
	                memset(fd, 0, sizeof(*fd));
			msg->result = ESUCCESS;
			msg->ipc(msg->from, Send, sizeof(*msg));
			return;
		
		    default:
			;
		}
		/*
		 * Did we complete the request? If not, enqueue it.
		 */
		if (!result)
		{
		    requests.insertTail(new FileSystemMessage(*msg));
		}
	    }
	    else
	    {
		msg->result = ENODEV;
		msg->ipc(msg->from, Send, sizeof(*msg));
	    }
	}
	
	/**
	 * @brief Interrupt request handler.
	 *
	 * Invokes the interrupt callback function of
	 * each Device registered for the interrupt vector.
	 *
	 * @param msg Incoming message from the kernel.
	 * @see Device
	 * @see Device::interrupt
	 */
	void interruptHandler(InterruptMessage *msg)
	{
	    List<Device> *lst = interrupts[msg->vector];
	
	    /* Do we have any Devices with this interrupt vector? */
	    if (lst)
	    {
		/*
		 * Loop all Devices of interest. Invoke callback.
		 */
		for (ListIterator<Device> i(lst); i.hasNext(); i++)
		{
		    i.current()->interrupt(msg->vector);
		}
	    }
	    /*
	     * Retry any requests in the queue.
	     * Remove them once processed.
	     */
	    for (ListIterator<FileSystemMessage> i(&requests); i.hasNext(); i++)
	    {
		/* Only handle read/write operations. */
		switch (i.current()->action)
		{
		    case ReadFile:
		    
			if (performRead(i.current()))
			{
			    requests.remove(i.current());
			}
			break;
		    
		    case WriteFile:
		    
			if (performWrite(i.current()))
			{
			    requests.remove(i.current());
			}

		    default:
			;
		}
	    }
	}

	/**
	 * @brief Attempt to perform a read operation.
	 *
	 * @param msg Request message.
	 * @return True if the request has completed. False otherwise.
	 */
	bool performRead(FileSystemMessage *msg)
	{
	    Device *dev = devices[msg->deviceID.minor];
	
	    /* Allocate a temporary buffer. */
	    s8 *buffer = new s8[msg->size];

	    /*
	     * Perform the read operation using the underlying
	     * read() implementation of the Device.
	     */
	    if ((msg->result = dev->read(buffer, msg->size, msg->offset)) >= 0)
	    {
		/* Write the result into the process' buffer. */
	        msg->result = VMCopy(msg->from, Write, (Address) buffer,
				    (Address) msg->buffer, msg->result);
	    }
	    /* Update FileDescriptor and send a reply if processed. */
	    if (msg->result != EAGAIN)
	    {
		if (msg->result > 0)
		    getFileDescriptor(files, msg->from, msg->fd)->position += msg->result;
		msg->ipc(msg->from, Send, sizeof(*msg));
	    }
	    /* Release memory. And return. */
	    delete buffer;
	    return msg->result != EAGAIN;
	}

	/**
	 * @brief Attempt to perform a write operation.
	 *
	 * @param msg Request message.
	 * @return True if the request has completed. False otherwise.
	 */
	bool performWrite(FileSystemMessage *msg)
	{
	    Device *dev = devices[msg->deviceID.minor];

	    /* Allocate a temporary buffer. */
	    s8 *buffer = new s8[msg->size];

	    /* Obtain input bytes from the process' buffer. */
	    if ((msg->result = VMCopy(msg->from, Read, (Address) buffer,
				     (Address) msg->buffer, msg->size)) >= 0)
	    {
		/*
		 * Perform the write operation using the underlying
		 * write() implementation of the Device.
	    	 */
		msg->result = dev->write(buffer, msg->size, msg->offset);
	    }
	    /* Send a reply if processed. */
	    if (msg->result != EAGAIN)
	    {
		if (msg->result > 0)
		    getFileDescriptor(files, msg->from, msg->fd)->position += msg->result;
		msg->ipc(msg->from, Send, sizeof(*msg));
	    }
	    /* Release memory. And return. */
	    delete buffer;
	    return msg->result != EAGAIN;
	}
	
	/** Contains all Devices served by this DeviceServer. */
	Array<Device> devices;
	
	/**
	 * @brief Registers Devices using interrupts.
	 *
	 * An Array with Lists of Devices using the
	 * interrupt vector as index.
	 * 
	 * @see Array
	 * @see List
	 * @see Device
	 */
	Array<List<Device> > interrupts;

	/**
	 * @brief A List of pending I/O operations.
	 */
	List<FileSystemMessage> requests;
	
	/** Per-process File descriptors. */
        Array<Shared<FileDescriptor> > *files;
	
	/**
	 * @brief Prefix string used to create device files in /tmp.
	 *
	 * If prefix contains e.g. "foobar", then add() will
	 * attempt to create "/dev/foobar0". If that fails it tries
	 * "/dev/foobar1", "/dev/foobar2", and so on.
	 */
	const char *prefix;
	
	/** The type of device file to create. */
	FileType type;
	
	/** Access permissions on the device files. */
	FileMode mode;
};

#endif /* __DEVICE_SERVER_H */

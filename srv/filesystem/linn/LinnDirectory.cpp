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

#include <API/VMCopy.h>
#include "LinnDirectory.h"

LinnDirectory::LinnDirectory(LinnFileSystem *f,
			     LinnInode *i)
    : fs(f), inode(i)
{
    size = inode->size;
}

Error LinnDirectory::read(FileSystemMessage *msg)
{
    LinnSuperBlock *sb = fs->getSuperBlock();
    LinnDirectoryEntry dent;
    LinnInode *dInode;
    Size bytes = ZERO;
    Dirent tmp, *buf = (Dirent *) msg->buffer;
    Error e;

    /* Loop all direct blocks. */
    for (u64 blk = 0; blk < LINN_INODE_DIR_BLOCKS; blk++)
    {
	/* Read directory entries. */
	for (u64 ent = 0; ent < LINN_DIRENT_PER_BLOCK(sb); ent++)
	{
	    /* Calculate offset to read. */
	    u64 offset = (inode->block[blk] * sb->blockSize) +
		         (ent * sizeof(LinnDirectoryEntry));

	    /* Get the next entry. */
	    if (fs->getStorage()->read(offset, (u8 *) &dent,
				       sizeof(LinnDirectoryEntry)) < 0)
	    {
		return EACCES;
	    }
	    /* Can we read another entry? */
            if (bytes + sizeof(Dirent) <= msg->size)
            {
		/* Fill in the Dirent. */
		if (!(dInode = fs->getInode(dent.inode)))
		{
		    return EINVAL;
		}
		strlcpy(tmp.name, dent.name, LINN_DIRENT_NAME_LEN);
		tmp.type = (FileType) dInode->type;

		/* Copy to the remote process. */		    
                if ((e = VMCopy(msg->procID, Write, (Address) &tmp,
                               (Address) (buf++), sizeof(Dirent))) < 0)
                {
                    return e;
                }
                bytes += e;
            }
	    /* No more buffer space left. */
            else return EFAULT;
	}
    }
    /* All done. */
    msg->size = bytes;
    return ESUCCESS;
}

Error LinnDirectory::getEntry(LinnDirectoryEntry *dent, char *name)
{
    LinnSuperBlock *sb = fs->getSuperBlock();
    u64 offset;

    /* Loop all blocks. */
    for (u64 blk = 0; blk < LINN_INODE_NUM_BLOCKS(sb, inode); blk++)
    {
	/* Read directory entries. */
	for (u64 ent = 0; ent < LINN_DIRENT_PER_BLOCK(sb); ent++)
	{
	    /* Calculate offset to read. */
	    offset = (inode->block[blk] * sb->blockSize) +
		     (sizeof(LinnDirectoryEntry) * ent);

	    /* Get the next entry. */
	    if (fs->getStorage()->read(offset, (u8 *) dent,
				       sizeof(LinnDirectoryEntry)) < 0)
	    {
		return EACCES;
	    }
	    /* Is it the entry we are looking for? */
	    if (strcmp(name, dent->name) == 0)
	    {
		return ESUCCESS;
	    }
	}
    }
    /* Not found. */
    return ENOENT;
}

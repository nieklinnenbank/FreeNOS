/*
 * Copyright (C) 2020 Niek Linnenbank
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

#include <BufferedFile.h>
#include <Lz4Decompressor.h>
#include "Decompress.h"

Decompress::Decompress(int argc, char **argv)
    : POSIXApplication(argc, argv)
{
    parser().setDescription("Decompress a compressed file");
    parser().registerPositional("FILE", "file(s) to decompress", 0);
}

Decompress::~Decompress()
{
}

Decompress::Result Decompress::exec()
{
    Result result = Success, ret = Success;
    const Vector<Argument *> & positionals = arguments().getPositionals();

    // Decompress all given files. */
    for (Size i = 0; i < positionals.count(); i++)
    {
        // Decompress the file. */
        result = decompressFile(*(positionals[i]->getValue()));

        // Update exit code if needed
        if (result != Success)
        {
            ret = result;
        }
    }
    // Done
    return ret;
}

Decompress::Result Decompress::decompressFile(const String inputFilename) const
{
    const char *lz4Extension = ".lz4";

    DEBUG("file = " << *inputFilename);

    // File must have the LZ4 extension
    if (!inputFilename.endsWith(lz4Extension))
    {
        ERROR("input file name does not end with " << lz4Extension);
        return InvalidArgument;
    }

    // Determine output name (without extension)
    const String outputFilename =
        inputFilename.substring(0, inputFilename.length() - String::length(lz4Extension));

    // Create buffered files
    BufferedFile input(*inputFilename);
    BufferedFile output(*outputFilename);

    // Read the input file
    if (input.read() != BufferedFile::Success)
    {
        ERROR("failed to read input file " << input.path());
        return IOError;
    }

    // Initialize decompressor
    Lz4Decompressor lz4(input.buffer(), input.size());
    const Lz4Decompressor::Result result = lz4.initialize();
    if (result != Lz4Decompressor::Success)
    {
        ERROR("failed to initialize LZ4 decompressor: result = " << (int) result);
        return IOError;
    }

    // Allocate temporary buffer
    const Size originalSize = lz4.getUncompressedSize();
    u8 *uncompressed = new u8[originalSize];

    // Decompress entire file
    const Lz4Decompressor::Result readResult = lz4.read(uncompressed, originalSize);
    if (readResult != Lz4Decompressor::Success)
    {
        ERROR("failed to decompress file " << *inputFilename << ": result = " << (int) readResult);
        delete[] uncompressed;
        return IOError;
    }

    // Write to output file
    const BufferedFile::Result writeResult = output.write(uncompressed, originalSize);
    if (writeResult != BufferedFile::Success)
    {
        ERROR("failed to write output file " << *outputFilename << ": result = " << (int) writeResult);
        delete[] uncompressed;
        return IOError;
    }

    // Cleanup resources
    delete[] uncompressed;
    return Success;
}

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

#include <FreeNOS/Config.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <BufferedFile.h>
#include <Lz4Decompressor.h>

#ifdef __HOST__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#endif /* __HOST__ */

TestCase(Lz4Construct)
{
    const u8 data[] = { 0 };

    Lz4Decompressor lz4(&data, sizeof(data));
    testAssert(lz4.m_inputData == data);
    testAssert(lz4.m_inputSize == sizeof(data));

    return OK;
}

TestCase(Lz4InitializeSize)
{
    const u32 data[8] = { 0x184D2204, 0x00004068, 0x12340000, 0x0 };

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // No input data
    Lz4Decompressor lz4_zero(&data, 0);
    testAssert(lz4_zero.initialize() != Lz4Decompressor::Success);

    // Minimum total input size is 27 bytes: 4 bytes magic + 15 bytes frame
    // descriptor + 4 bytes for the EndMarker + 4 bytes content checksum.
    Lz4Decompressor lz4_invalid(&data, 26);
    testAssert(lz4_invalid.initialize() != Lz4Decompressor::Success);

    Lz4Decompressor lz4_valid(&data, 27);
    testAssert(lz4_valid.initialize() == Lz4Decompressor::Success);

    return OK;
}

TestCase(Lz4InitializeMagic)
{
    const u32 invalid[8] = { 0x12345678, 0x00004068, 0x12340000, 0x0 };
    const u32 valid[8]   = { 0x184D2204, 0x00004068, 0x12340000, 0x0 };

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Invalid magic
    Lz4Decompressor lz4_invalid(&invalid, sizeof(invalid));
    testAssert(lz4_invalid.initialize() != Lz4Decompressor::Success);

    // Valid magic
    Lz4Decompressor lz4_valid(&valid, sizeof(valid));
    testAssert(lz4_valid.initialize() == Lz4Decompressor::Success);

    return OK;
}

TestCase(Lz4InitializeFrameVersion)
{
    const u32 invalid[8] = { 0x184D2204, 0x000040a8, 0x12340000, 0x0 };
    const u32 valid[8]   = { 0x184D2204, 0x00004068, 0x12340000, 0x0 };

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Invalid frame version
    Lz4Decompressor lz4_invalid(&invalid, sizeof(invalid));
    testAssert(lz4_invalid.initialize() != Lz4Decompressor::Success);

    // Valid frame version
    Lz4Decompressor lz4_valid(&valid, sizeof(valid));
    testAssert(lz4_valid.initialize() == Lz4Decompressor::Success);

    return OK;
}

TestCase(Lz4InitializeBlockChecksum)
{
    const u32 enabled[8]  = { 0x184D2204, 0x00004078, 0x12340000, 0x0 };
    const u32 disabled[8] = { 0x184D2204, 0x00004068, 0x12340000, 0x0 };

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Block checksum enabled
    Lz4Decompressor lz4_enabled(&enabled, sizeof(enabled));
    testAssert(lz4_enabled.initialize() == Lz4Decompressor::Success);
    testAssert(lz4_enabled.m_blockChecksums);

    // Block checksum disabled
    Lz4Decompressor lz4_disabled(&disabled, sizeof(disabled));
    testAssert(lz4_disabled.initialize() == Lz4Decompressor::Success);
    testAssert(!lz4_disabled.m_blockChecksums);

    return OK;
}

TestCase(Lz4InitializeBlockDep)
{
    const u32 invalid[8] = { 0x184D2204, 0x00004048, 0x12340000, 0x0 };
    const u32 valid[8]   = { 0x184D2204, 0x00004068, 0x12340000, 0x0 };

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Dependent blocks
    Lz4Decompressor lz4_invalid(&invalid, sizeof(invalid));
    testAssert(lz4_invalid.initialize() != Lz4Decompressor::Success);

    // Independent blocks
    Lz4Decompressor lz4_valid(&valid, sizeof(valid));
    testAssert(lz4_valid.initialize() == Lz4Decompressor::Success);

    return OK;
}

TestCase(Lz4InitializeBlockMaxSize)
{
    const u32 invalid[8]     = { 0x184D2204, 0x00003068, 0x12340000, 0x0 };
    const u32 valid_64kb[8]  = { 0x184D2204, 0x00004068, 0x12340000, 0x0 };
    const u32 valid_256kb[8] = { 0x184D2204, 0x00005068, 0x12340000, 0x0 };
    const u32 valid_1mb[8]   = { 0x184D2204, 0x00006068, 0x12340000, 0x0 };

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Call initialize with invalid maximum block size
    Lz4Decompressor lz4_invalid(&invalid, sizeof(invalid));
    testAssert(lz4_invalid.initialize() != Lz4Decompressor::Success);

    // 64KB max block size
    Lz4Decompressor lz4_64kb(&valid_64kb, sizeof(valid_64kb));
    testAssert(lz4_64kb.initialize() == Lz4Decompressor::Success);
    testAssert(lz4_64kb.m_blockMaximumSize == 64 * 1024);

    // 256KB max block size
    Lz4Decompressor lz4_256kb(&valid_256kb, sizeof(valid_256kb));
    testAssert(lz4_256kb.initialize() == Lz4Decompressor::Success);
    testAssert(lz4_256kb.m_blockMaximumSize == 256 * 1024);

    // 1MB max block size
    Lz4Decompressor lz4_1mb(&valid_1mb, sizeof(valid_1mb));
    testAssert(lz4_1mb.initialize() == Lz4Decompressor::Success);
    testAssert(lz4_1mb.m_blockMaximumSize == 1024 * 1024);

    return OK;
}

TestCase(Lz4InitializeContentSize)
{
    const u32 disabled[8] = { 0x184D2204, 0x00004060, 0x0, 0x0 };
    const u32 enabled[8]  = { 0x184D2204, 0x56784068, 0x00001234, 0x0 };

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Content size must be given
    Lz4Decompressor lz4_disabled(&disabled, sizeof(disabled));
    testAssert(lz4_disabled.initialize() != Lz4Decompressor::Success);

    // Content size enabled
    Lz4Decompressor lz4_enabled(&enabled, sizeof(enabled));
    testAssert(lz4_enabled.initialize() == Lz4Decompressor::Success);
    testAssert(lz4_enabled.getUncompressedSize() == 0x12345678);

    return OK;
}

TestCase(Lz4InitializeContentChecksum)
{
    const u32 enabled[8]  = { 0x184D2204, 0x0000406c, 0x12340000, 0x0 };
    const u32 disabled[8] = { 0x184D2204, 0x00004068, 0x12340000, 0x0 };

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Checksum disabled
    Lz4Decompressor lz4_disabled(&disabled, sizeof(disabled));
    testAssert(lz4_disabled.initialize() == Lz4Decompressor::Success);
    testAssert(!lz4_disabled.m_contentChecksum);

    // Checksum enabled. Note that the current implementation
    // does not actually verify the checksum
    Lz4Decompressor lz4_enabled(&enabled, sizeof(enabled));
    testAssert(lz4_enabled.initialize() == Lz4Decompressor::Success);
    testAssert(lz4_enabled.m_contentChecksum);

    return OK;
}

TestResult Lz4DecompressFileTest(const char *original, const char *compressed)
{
    BufferedFile originalFile(original);
    BufferedFile compressedFile(compressed);

    // Do not mask error output
    Log::instance()->setMinimumLogLevel(Log::Notice);

    // Read original file
    if (originalFile.read() != BufferedFile::Success)
    {
        ERROR("failed to read input file " << originalFile.path());
        return SKIP;
    }

    // Read compressed file
    if (compressedFile.read() != BufferedFile::Success)
    {
        ERROR("failed to read input file " << compressedFile.path());
        return SKIP;
    }

    // Initialize decompressor
    Lz4Decompressor lz4(compressedFile.buffer(), compressedFile.size());
    testAssert(lz4.initialize() == Lz4Decompressor::Success);

    // Allocate temporary buffer
    u8 *uncompressed = new u8[originalFile.size() + 1];
    MemoryBlock::set(uncompressed, 0, originalFile.size() + 1);

    // Decompress entire file
    testAssert(lz4.read(uncompressed, originalFile.size()) == Lz4Decompressor::Success);

    // Compare with the original
    for (Size i = 0; i < originalFile.size(); i++)
    {
        DEBUG("i = " << i << " uncompressed = " << (char) uncompressed[i] <<
              " original = " << (char) ((u8 *)(originalFile.buffer()))[i]);
        testAssert(uncompressed[i] == ((u8 *)(originalFile.buffer()))[i]);
    }

    delete[] uncompressed;
    return OK;
}

TestCase(Lz4DecompressConfig)
{
    return Lz4DecompressFileTest(TESTROOT "/etc/Config.h",
                                 TESTROOT "/etc/Config.h.lz4");
}

#ifdef __HOST__
TestCase(Lz4DecompressBootImage)
{
    return Lz4DecompressFileTest(TESTROOT "/boot.img",
                                 TESTROOT "/boot.img.lz4");
}

TestCase(Lz4DecompressRandom)
{
    // Generate a variable size random data file
    TestInt<Size> sizes(KiloByte(16), MegaByte(1));
    const Size fileSize = sizes.random();
    const char *orig = "testdata.bin";
    const char *comp = "testdata.bin.lz4";

    // Open temporary file for writing
    FILE *fp = fopen(orig, "w+");
    if (fp == NULL)
    {
        ERROR("failed to open output file " << orig << ": " << strerror(errno));
        return FAIL;
    }

    // Generate content
    u8 *data = new u8[fileSize];
    u8 value = random();
    for (Size i = 0; i < fileSize; i++)
    {
        // Decrease randomness to get more realistic compression ratio
        if ((i % 0xff) <= value)
        {
            value = random();
        }
        data[i] = value;
    }

    // Write to the file
    if (fwrite(data, fileSize, 1, fp) != 1)
    {
        ERROR("failed to write output file " << orig << ": " << strerror(errno));
        fclose(fp);
        unlink(orig);
        delete[] data;
        return FAIL;
    }
    fclose(fp);

    // Compress the file with LZ4
    String command;
    command << "lz4 -q --content-size " << orig << " " << comp;
    const int ret = system(*command);
    if (ret != 0)
    {
        ERROR("failed to compress output file " << orig << " -> " << comp <<
              ": ret = " << ret);
        delete[] data;
        unlink(orig);
        return FAIL;
    }

    // Run decompression test
    const TestResult result = Lz4DecompressFileTest(orig, comp);

    // Cleanup resources
    delete[] data;
    unlink(orig);
    unlink(comp);
    return result;
}
#endif /* __HOST__ */

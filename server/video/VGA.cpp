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

#include <FreeNOS/System.h>
#include <Types.h>
#include <MemoryBlock.h>
#include "VGA.h"

VGA::VGA(Size w, Size h) : Device(FileSystem::BlockDeviceFile), width(w), height(h)
{
    m_identifier << "vga0";
}

FileSystem::Error VGA::initialize()
{
    Memory::Range range;

    // Request VGA memory
    range.size   = PAGESIZE;
    range.access = Memory::User     |
                   Memory::Readable |
                   Memory::Writable;
    range.virt   = ZERO;
    range.phys   = VGA_PADDR;
    VMCtl(SELF, Map, &range);

    // Point to the VGA mapping
    vga = (u16 *) range.virt;

    // Clear screen
    for (uint i = 0; i < width * height; i++)
    {
        vga[i] = VGA_CHAR(' ', LIGHTGREY, BLACK);
    }

    // Disable hardware cursor
    m_io.outb(VGA_IOADDR, 0x0a);
    m_io.outb(VGA_IODATA, 1 << 5);

    // Successfull
    return FileSystem::Success;
}

FileSystem::Error VGA::read(IOBuffer & buffer, Size size, Size offset)
{
    if (offset + size > width * height * sizeof(u16))
    {
        return FileSystem::InvalidArgument;
    }
    buffer.write(vga + (offset / sizeof(u16)), size);
    return size;
}

FileSystem::Error VGA::write(IOBuffer & buffer, Size size, Size offset)
{
    if (offset + size > width * height * sizeof(u16))
    {
        return FileSystem::InvalidArgument;
    }

    MemoryBlock::copy(vga + (offset / sizeof(u16)), buffer.getBuffer(), size);
    return size;
}

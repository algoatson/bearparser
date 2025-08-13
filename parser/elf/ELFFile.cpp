#include "elf/ELFFile.h"
#include "FileBuffer.h"

bool ELFFileBuilder::signatureMatches(AbstractByteBuffer *buf)
{
    if (buf == NULL) return false;

    const char *magic = (const char*) buf->getContentAt(0, 4);
    if (magic == NULL) return false;

    // Check for the ELF magic number
    if (magic[0] == '\x7F' && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F') {
        return true;
    }

    return false;
}

Executable* ELFFileBuilder::build(AbstractByteBuffer *buf)
{
    Executable *exe = NULL;
    if (signatureMatches(buf) == false) return NULL;

    try {
        exe = new ELFFile(buf);
    } catch (ExeException) {
        exe = nullptr;
    }

    return exe;
}

ELFFile::ELFFile(AbstractByteBuffer *v_buf)
    : MappedExe(v_buf, Executable::BITS_64) // default should be 64-bit.
      // elfHdr(NULL), progHdrs(NULL), sectHdrs(NULL), symTab(NULL), dynTab(NULL)
{
    clearWrappers();

    _init(v_buf);
    Logger::append(Logger::D_INFO, "Wrapped");
}

void ELFFile::_init(AbstractByteBuffer *v_buf) 
{
    core.wrap(v_buf);
    this->bitMode = core.getHdrBitMode();

    this->elfHdr = new ElfHdrWrapper(this);
    this->progHdrs = new ElfProgHdrWrapper(this);
    this->sectHdrs = new ElfSectHdrWrapper(this);
    // this->symTab = new ElfSymTabWrapper(this);
    // this->dynTab = new ElfDynWrapper(this);
}

void ELFFile::clearWrappers() {
    MappedExe::clearWrappers();
    this->elfHdr   = NULL;
    this->progHdrs = NULL;
    // this->SectHdrs = NULL;
    // this->symTab   = NULL;
    // this->dynTab   = NULL;
}

offset_t ELFFile::getEntryPoint(Executable::addr_type addrType) {
    // need to do some verifications here.
    // if (addrType != Executable::VA) {
    //     Logger::append(Logger::D_ERROR, "ELF entry point must be VA.");
    //     return INVALID_ADDR;
    // }

    return core.getEntryPoint();
}

bufsize_t ELFFile::getMappedSize(Executable::addr_type aType) {
    if (aType == Executable::NOT_ADDR) return 0;

    if (aType == Executable::RAW) {
        return core.getRawSize();
    }

    constexpr size_t unit_size = 0x1000;

    if (aType == Executable::VA || aType == Executable::RVA) {
        bufsize_t vSize = core.getVirtualSize();
        return (vSize < unit_size) ? unit_size : vSize;
    }

    return 0;
}
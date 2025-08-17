#pragma once

#include "ELFCore.h"

#include "ElfHdrWrapper.h"
#include "ElfProgHdrWrapper.h"
#include "ElfSectHdrWrapper.h"
#include "ElfSymTabWrapper.h"
#include "ElfDynWrapper.h"

#include "../MappedExe.h"
#include <QDebug>

class ELFFileBuilder: public ExeBuilder {
public:
    ELFFileBuilder() : ExeBuilder() {
        // qDebug() << "ELFFileBuilder initialized";
    }

    virtual ~ELFFileBuilder() {
        // qDebug() << "ELFFileBuilder destroyed";
    }

    virtual bool signatureMatches(AbstractByteBuffer *buf);
    virtual Executable* build(AbstractByteBuffer *buf);
    QString typeName() { return "ELF"; }
};

class ELFFile : public MappedExe
{
public:
    enum WRAPPERS {
        WR_NONE = MappedExe::WR_NONE,
        WR_ELF_HDR = 0,
        WR_PROG_HDRS,
        WR_SECTION_HDRS,
        WR_SYMBOL_TABLE,
        WR_DYN_SYM_TABLE,
        FIELD_COUNTER
    };

    ELFFile(AbstractByteBuffer *v_buf);

    virtual void wrap() { return; }  // inherited from Executable
    
    virtual bufsize_t getMappedSize(Executable::addr_type aType);
    virtual bufsize_t getAlignment(Executable::addr_type aType) const { return core.cacheAlignment(); }
    virtual offset_t getImageBase(bool recalculate = false) { return core.cacheImageBase(); }
    virtual offset_t getEntryPoint(Executable::addr_type addrType = Executable::RVA); // returns INVALID_ADDR if failed
    virtual offset_t rawToRva(offset_t raw) { return 0; }
    virtual offset_t rvaToRaw(offset_t rva) { return 0; }
    

    std::variant<Elf32_Ehdr*, Elf64_Ehdr*> getEhdrVariant() const {
        return core.getEhdrVariant();
    }

    QVector<std::variant<Elf32_Phdr*, Elf64_Phdr*>> getProgramHeaders() const {
        return core.getProgramHeaders();
    }

    QVector<std::variant<Elf32_Shdr*, Elf64_Shdr*>> getSectionHeaders() const {
        return core.getSectionHeaders();
    }
    
    // std::variant<Elf32_Phdr*, Elf64_Phdr*> getPhdrsVariant() const {
    //     return core.getPhdrsVariant();
    // }

    // std::variant<Elf32_Shdr*, Elf64_Shdr*> getShdrsVariant() const {
    //     return core.getShdrsVariant();
    // }

    virtual exe_bits getHdrBitMode() { return core.getHdrBitMode(); }
    virtual exe_arch getArch() { return ARCH_UNKNOWN; }

    //---
    // ELFFile only:
    offset_t elfProgHdrOffset() const { return core.getProgramHdrsOffset(); }
    bufsize_t elfProgHdrCount() const { return core.getProgramHdrsCount(); }
    bufsize_t elfProgHdrSize()  const { return core.getProgramHdrsSize(); }
    
    offset_t elfSectHdrOffset() const { return core.getSectionHdrsOffset(); }
    bufsize_t elfSectHdrSize()  const { return core.getSectionHdrsSize(); }

protected:
    void _init(AbstractByteBuffer *v_buf);
    virtual void clearWrappers();

    ELFCore core;

    ElfHdrWrapper *elfHdr;
    ElfProgHdrWrapper *progHdrs;
    ElfSectHdrWrapper *sectHdrs;
    // ElfSymTabWrapper *symTab;
    // ElfDynWrapper *dynTab;
};

// class ELFFile : public MappedExe {
// public:
//     enum WRAPPERS {
//         WR_NONE = MappedExe::WR_NONE,
//         WR_ELF_HDR = 0,
//         WR_SECTION_HDRS,
//         WR_PROGRAM_HDRS,
//         WR_SYMBOL_TABLE,
//         WR_STR_TABLE,
//         COUNT_WRAPPERS
//     };

//     static long computeChecksum(const BYTE *buffer, size_t bufferSize, offset_t checksumOffset);
// public:
//     ELFFile(AbstractByteBuffer *v_buf);
//     virtual ~ELFFile() {}
// };

// ELFFile::ELFFile(AbstractByteBuffer *v_buf)
//     : MappedExe(v_buf, Executable::BITS_64) {
//     qDebug() << "ELFFile created";
// }
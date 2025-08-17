#pragma once

#include "elf/ELFNodeWrapper.h"
#include "elf.h"

#include <QDebug>

class ELFFile; // forward declaration

class ElfSectHdrWrapper : public ELFElementWrapper
{
public:
    enum FieldID {
        NONE = -1,
        SH_NAME = 0,
        SH_TYPE,
        SH_FLAGS,
        SH_ADDR,
        SH_OFFSET,
        SH_SIZE,
        SH_LINK,
        SH_INFO,
        SH_ADDROFFSET,
        SH_ENTSIZE,
        SH_ADDRALIGN,
        SH_FLAGS2,
        FIELD_COUNTER
    };

    ElfSectHdrWrapper(ELFFile *elfExe);

    bool wrap();
    virtual void* getPtr();

    virtual bufsize_t getEntrySize();
    virtual bufsize_t getSize();

    virtual QString getName() {
        return "ELF Section Header";
    }

    virtual size_t getFieldsCount() { return FIELD_COUNTER; }

    virtual void *getFieldPtr(size_t fieldId, size_t subField = FIELD_NONE) { }
    virtual QString getFieldName(size_t fieldId) { }
    virtual Executable::addr_type containsAddrType(size_t fieldId, size_t subField = FIELD_NONE) { }
protected:
    QVector<std::variant<Elf32_Shdr*, Elf64_Shdr*>> shdrs;
};
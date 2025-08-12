#pragma once

#include "elf/ELFNodeWrapper.h"
#include "elf.h"

#include <QDebug>

class ELFFile; // forward declaration

class ElfProgHdrWrapper : public ELFElementWrapper
{
public:
    enum FieldID {
        NONE = FIELD_NONE,
        PH_TYPE = 0,
        PH_FLAGS,
        PH_OFFSET,
        PH_VADDR,
        PH_PADDR,
        PH_FILE_SIZE,
        PH_MEM_SIZE,
        PH_ALIGN,
        FIELD_COUNTER
    };

    ElfProgHdrWrapper(ELFFile *elfExe) 
        : ELFElementWrapper(elfExe),
          phdrs(static_cast<Elf64_Phdr*>(nullptr)) 
        {
            qInfo() << "Program Headers Entry Size:" << getEntrySize();
            qInfo() << "Program Headers Size:" << getSize();
            wrap();

            void *ptr = getPtr();
            qInfo() << "ELF Program Header is located at:" << ptr;
        }

    bool wrap();
    virtual void* getPtr();

    virtual bufsize_t getEntrySize();
    virtual bufsize_t getSize();

    virtual QString getName() {
        return "ELF Program Header";
    }

    virtual size_t getFieldsCount() { return FIELD_COUNTER; }

    virtual void* getFieldPtr(size_t fieldId, size_t subField = FIELD_NONE) { }
    virtual QString getFieldName(size_t fieldId) { }
    virtual Executable::addr_type containsAddrType(size_t fieldId, size_t subField = FIELD_NONE) { }
protected:
    std::variant<Elf32_Phdr*, Elf64_Phdr*> phdrs;

};
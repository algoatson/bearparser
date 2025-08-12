#pragma once

#include "elf/ELFNodeWrapper.h"
#include "elf.h"

#include <QDebug>
#include <unordered_map>

class ELFFile; // forward declaration

class ElfHdrWrapper : public ELFElementWrapper
{
public:
    enum FieldID {
        NONE = FIELD_NONE,
        E_IDENT_MAG = 0,
        E_IDENT_CLASS,
        E_IDENT_DATA,
        E_IDENT_VERSION,
        E_IDENT_OSABI,
        E_IDENT_ABIVERSION,
        FIELD_COUNTER
    };

    static const std::unordered_map<uint16_t, QString> s_machine;

    ElfHdrWrapper(ELFFile *elfExe) 
        : ELFElementWrapper(elfExe), 
          hdr(static_cast<Elf64_Ehdr*>(nullptr)) {
            qInfo() << "ELF Header Size:" << getSize();
            wrap();
            
            if (std::holds_alternative<Elf64_Ehdr*>(hdr)) {
                qInfo() << "ELF64 Header is used.";
            } else if (std::holds_alternative<Elf32_Ehdr*>(hdr)) {
                qInfo() << "ELF32 Header is used.";
            } else {
                qWarning() << "Unknown ELF header type!";
            }

            void *ptr = getPtr();
            qInfo() << "ELF Header is located at:" << ptr;
    }

    bool wrap();

    virtual void* getPtr();

    // should return the size of the ELF header
    virtual bufsize_t getSize();

    virtual QString getName() { return "ELF Hdr"; }
    virtual size_t getFieldsCount() { return FIELD_COUNTER; }

    virtual void* getFieldPtr(size_t fieldId, size_t subField = FIELD_NONE);
    virtual QString getFieldName(size_t fieldId) { }
    virtual Executable::addr_type containsAddrType(size_t fieldId, size_t subField = FIELD_NONE) { }
private:
    std::variant<Elf32_Ehdr*, Elf64_Ehdr*> hdr;

};
#pragma once
#include <map>
#include <QMap>

#include "AbstractByteBuffer.h"
class Executable;

class ExeException : public CustomException
{
public:
    ExeException(const QString info) : CustomException(info) {}
};

class ExeBuilder {
public:
    ExeBuilder() {}
    virtual ~ExeBuilder() {}

    virtual bool signatureMatches(AbstractByteBuffer *buf) = 0;
    virtual Executable* build(AbstractByteBuffer *buf) = 0;
    virtual QString typeName() = 0;
};

//-------------------------------------------------------------

class Executable : public AbstractByteBuffer {
public:
    enum exe_bits {
        UNKNOWN = 0,
        BITS_16 = 16,
        BITS_32 = 32,
        BITS_64 = 64,
    };
	
	enum exe_arch {
		ARCH_UNKNOWN = 0,
		ARCH_INTEL = 1,
		ARCH_ARM = 2
	};

    enum addr_type {
        NOT_ADDR = 0,
        RAW = 1,
        RVA = 2,
        VA = 3
    };
    static bool isBit64(Executable *exe) { return (!exe || exe->getBitMode() != Executable::BITS_64) ? false: true; }
    static bool isBit32(Executable *exe) { return (!exe || exe->getBitMode() != Executable::BITS_32) ? false: true; }

    bool isBit64() { return isBit64(this); }
    bool isBit32() { return isBit32(this); }

    virtual ~Executable(void) { }

    virtual exe_bits getBitMode() { return this->bitMode; }

    virtual bufsize_t getContentSize() { return buf->getContentSize(); }
    virtual BYTE* getContent() { return buf->getContent(); }
    //wrapper:
    virtual offset_t getRawSize() const { return static_cast<offset_t>(buf->getContentSize()); }

    BYTE* getContentAtPtr(BYTE* ptr, bufsize_t size, bool allowExceptions = false) { return AbstractByteBuffer::getContentAtPtr(ptr, size, allowExceptions); }
    BYTE* getContentAt(offset_t offset, bufsize_t size, bool allowExceptions = false) { return AbstractByteBuffer::getContentAt(offset, size, allowExceptions); }

    virtual BYTE* getContentAt(offset_t offset, Executable::addr_type aType, bufsize_t size, bool allowExceptions = false);
//------------------------------
    virtual bufsize_t getMappedSize(Executable::addr_type aType) = 0;
    virtual bufsize_t getAlignment(Executable::addr_type aType) const = 0;
    virtual offset_t getImageBase(bool recalculate = false) = 0;
    virtual offset_t getEntryPoint(Executable::addr_type aType = Executable::RVA) = 0;
    
    virtual bufsize_t getImageSize() { return getMappedSize(Executable::VA); }
    
    /* All Entry Points of the application, including: main EP, Exports, TLS Callbacks */
    virtual size_t getAllEntryPoints(QMap<offset_t,QString> &entrypoints, Executable::addr_type aType = Executable::RVA) 
    {
        offset_t mainEP = getEntryPoint(aType);
        entrypoints.insert(mainEP, "_start");
        return 1;
    }

    /* conversions */
    virtual bool isValidAddr(offset_t addr, addr_type addrType);
    virtual bool isValidVA(offset_t va) { return isValidAddr(va, Executable::VA); }

    virtual offset_t convertAddr(offset_t inAddr, Executable::addr_type inType, Executable::addr_type outType);

    virtual offset_t toRaw(offset_t offset, addr_type addrType, bool allowExceptions = false); //any type of offset to raw
    Executable::addr_type detectAddrType(offset_t addr, Executable::addr_type hintType); //TODO

    // returns INVALID_ADDR if failed
    // FileAddr <-> RVA
    virtual offset_t rawToRva(offset_t raw) = 0;
    virtual offset_t rvaToRaw(offset_t rva) = 0;

    // VA <-> RVA
    virtual offset_t VaToRva(offset_t va, bool autodetect = false);

    virtual offset_t rvaToVa(offset_t rva)
    {
        return (rva == INVALID_ADDR) ? INVALID_ADDR : (rva + this->getImageBase());
    }

    // VA -> FileAddr
    virtual offset_t vaToRaw(offset_t va)
    {
        if (va == INVALID_ADDR) return INVALID_ADDR;

        offset_t rva = this->VaToRva(va, true);
        return rvaToRaw(rva);
    }

    QString getFileName();

    virtual bool resize(bufsize_t newSize) { return buf->resize(newSize); }

    virtual bool isResized() { return buf ? buf->isResized() : false; }

    virtual bool isTruncated() { return buf ? buf->isTruncated() : false; }

    /* wrappers */
    AbstractByteBuffer* getFileBuffer() const { return buf; }
    bufsize_t getFileSize() const;

    virtual bool dumpFragment(offset_t offset, bufsize_t size, QString fileName);

protected:
    Executable(AbstractByteBuffer *v_buf, exe_bits v_bitMode);

    exe_bits bitMode;
    AbstractByteBuffer *buf;
};


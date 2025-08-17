#pragma once

#include "../Executable.h"
#include "elf.h"
#include <variant>
#include <QVector>

// Class for internal use of ELFFile
class ELFCore
{
public:

    ELFCore() :
        buf(nullptr),
        ehdr(static_cast<Elf64_Ehdr*>(nullptr)), 
        phdrs(static_cast<Elf32_Phdr*>(nullptr)), 
        shdrs(static_cast<Elf32_Shdr*>(nullptr)) 
    {}

    virtual ~ELFCore() { reset(); }

    bool wrap(AbstractByteBuffer *v_buf);

    // Header info
    virtual offset_t getEntryPoint() const;
    virtual offset_t getRawSize() const;
    virtual offset_t getImageBase() const;
    virtual bufsize_t getVirtualSize() const;
    virtual bufsize_t getImageSize() const;
    virtual bufsize_t getAlignment() const;
    Executable::exe_bits getHdrBitMode() const;
    Executable::exe_arch getHdrArch() const;

    // Header counts
    size_t getProgramHdrsCount()    const;
    size_t getSectionHdrsCount()    const;

    // Header offsets and sizes
    offset_t getProgramHdrsOffset() const;
    bufsize_t getProgramHdrsSize()  const;
    offset_t getSectionHdrsOffset() const;
    bufsize_t getSectionHdrsSize()  const;

    // Headers access by index
    offset_t getProgramHdrByIndex(int idx) const;
    offset_t getSectionHdrByIndex(int idx) const;
    bufsize_t getSectionHdrSizeByIndex(int idx)  const;
    bufsize_t getProgramHdrSizeByIndex(int idx)  const;

    // High-level querying
    QVector<std::variant<Elf32_Phdr*, Elf64_Phdr*>> getProgramHeaders() const;
    QVector<std::variant<Elf32_Shdr*, Elf64_Shdr*>> getSectionHeaders() const;

    int findSectionByName(const QString& name) const;
    int findSegmentByType(uint32_t type) const;
    int findSectionByType(uint32_t type) const;
    QString getSectionNameByIndex(int idx) const;

    // Flags
    bool isLoadableSegment(int idx) const;
    bool isExecutableSegment(int idx) const;
    bool sectionHasFlag(int idx, uint32_t flag) const;
    bool segmentHasFlag(int idx, uint32_t flag) const;

private:
    // Internal helpers
    template <typename EhdrT, typename PhdrT, typename ShdrT>
    bool wrapElfHeaders(AbstractByteBuffer* buf, bool allowExceptionsFromBuffer);

    template <typename T> T* getElfHeader() const;
    template <typename T> T* getProgramHeaders() const;
    template <typename T> T* getSectionHeaders() const;
    
    // Cache
    offset_t cacheImageBase()    const;
    bufsize_t cacheVirtualSize() const;
    bufsize_t cacheImageSize()   const;
    bufsize_t cacheAlignment()   const;
    QVector<QString> cacheSectionNames() const;

    // Variant helpers
    std::variant<Elf32_Ehdr*, Elf64_Ehdr*> getEhdrVariant() const;
    template<typename EhdrT> std::variant<Elf32_Ehdr*, Elf64_Ehdr*> getEhdrVariantT() const;
    
    std::variant<Elf32_Phdr*, Elf64_Phdr*> getPhdrsVariant() const;
    template<typename PhdrT> std::variant<Elf32_Phdr*, Elf64_Phdr*> getPhdrsVariantT() const;

    std::variant<Elf32_Shdr*, Elf64_Shdr*> getShdrsVariant() const;
    template<typename ShdrT> std::variant<Elf32_Shdr*, Elf64_Shdr*> getShdrsVariantT() const;

protected:
    void reset();
    // this field has become almost useless, since we templated everything.
    bool is64() const { return std::holds_alternative<Elf64_Ehdr*>(ehdr); }

private:
    AbstractByteBuffer *buf;

    std::variant<Elf32_Ehdr*, Elf64_Ehdr*> ehdr;
    std::variant<Elf32_Phdr*, Elf64_Phdr*> phdrs;
    std::variant<Elf32_Shdr*, Elf64_Shdr*> shdrs;

    QVector<std::variant<Elf32_Phdr*, Elf64_Phdr*>> __phdrs;
    QVector<std::variant<Elf32_Shdr*, Elf64_Shdr*>> __shdrs;

    // Caching
    mutable offset_t cachedImageSize  = 0;
    mutable bool cachedImageSizeValid = false;

    mutable offset_t cachedImageBase  = UINT64_MAX;
    mutable bool cachedImageBaseValid = false;
    
    mutable bufsize_t cachedVirtualSize = 0;
    mutable bool cachedVirtualSizeValid = false;

    mutable bufsize_t cachedAlignment = 0;
    mutable bool cachedAlignmentValid = false;

    mutable QVector<QString> cachedSectionNames;
    mutable bool cachedSectionNamesValid = false;

friend class ELFFile;
};
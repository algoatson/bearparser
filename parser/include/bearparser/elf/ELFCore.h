#pragma once

#include "../Executable.h"
#include "elf.h"
#include <variant>

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

    virtual ~ELFCore() { /*reset();*/ }

    bool wrap(AbstractByteBuffer *v_buf);

    template <typename EhdrT, typename PhdrT, typename ShdrT>
    bool wrapElfHeaders(AbstractByteBuffer* buf, bool allowExceptionsFromBuffer);

    virtual offset_t getEntryPoint() const;
    virtual offset_t getImageBase() const;
    virtual offset_t getRawSize() const { return buf ? static_cast<offset_t>(buf->getContentSize()) : 0; }
    virtual bufsize_t getVirtualSize() const;

    virtual bufsize_t getAlignment() const;
    virtual bufsize_t getImageSize() const;


    Executable::exe_bits getHdrBitMode() const;
    Executable::exe_arch getHdrArch() const;

    offset_t elfSectionHdrsOffset() const;
    offset_t elfProgramHdrsOffset() const;
    bufsize_t elfSectionHdrsSize()  const;
    bufsize_t elfProgramHdrsSize()  const;
    size_t elfProgramHdrsCount()    const;
    size_t elfSectionHdrsCount()    const;

private:
    // caching variables to avoid unecessary loops.
    mutable offset_t cachedImageSize  = 0;
    mutable bool cachedImageSizeValid = false;

    mutable offset_t cachedImageBase  = UINT64_MAX;
    mutable bool cachedImageBaseValid = false;

    // Getters for ELF Header
    template <typename T>
    T* getElfHeader() const;

    // Getters for Program Headers
    template <typename T>
    T* getProgramHeaders() const;

    // Getters for Section Headers
    template <typename T>
    T* getSectionHeaders() const;

protected:
    void reset();
    // this field has become almost useless, since we templated everything.
    bool is64() const { return std::holds_alternative<Elf64_Ehdr*>(ehdr); }

    AbstractByteBuffer *buf;

    // ELF Header
    std::variant<Elf32_Ehdr*, Elf64_Ehdr*> getEhdrVariant() const;

    template<typename EhdrT>
    std::variant<Elf32_Ehdr*, Elf64_Ehdr*> getEhdrVariantT() const;

    // Program Headers
    std::variant<Elf32_Phdr*, Elf64_Phdr*> getPhdrsVariant() const;

    template<typename PhdrT>
    std::variant<Elf32_Phdr*, Elf64_Phdr*> getPhdrsVariantT() const;

    // Section Headers
    std::variant<Elf32_Shdr*, Elf64_Shdr*> getShdrsVariant() const;
    
    template<typename ShdrT>
    std::variant<Elf32_Shdr*, Elf64_Shdr*> getShdrsVariantT() const;

    // Pointers to the ELF structures
    // Using std::variant to hold either Elf32 or Elf64 structures
    // This allows us to handle both 32-bit and 64-bit ELF files in a type-safe manner
    std::variant<Elf32_Ehdr*, Elf64_Ehdr*> ehdr;
    std::variant<Elf32_Phdr*, Elf64_Phdr*> phdrs;
    std::variant<Elf32_Shdr*, Elf64_Shdr*> shdrs;

friend class ELFFile;
};
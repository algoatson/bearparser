#include "elf/ELFCore.h"

template <typename... Ts>
bool isVariantNullptr(const std::variant<Ts...>& var) {
    return std::visit([](auto ptr) { return ptr == nullptr; }, var);
}

template <typename T>
T* getStructAt(AbstractByteBuffer* buf, size_t offset, size_t count, bool allowThrow = true) {
    return reinterpret_cast<T*>(buf->getContentAt(offset, count, allowThrow));
}

template <typename EhdrT, typename PhdrT, typename ShdrT>
bool ELFCore::wrapElfHeaders(AbstractByteBuffer* v_buf, bool allowExceptionsFromBuffer) {
    static_assert(std::is_same_v<EhdrT, Elf64_Ehdr> || std::is_same_v<EhdrT, Elf32_Ehdr>,
                  "Invalid type passed to wrapElfHeaders<EhdrT, PhdrT, ShdrT>() — must be Elf32_Ehdr or Elf64_Ehdr");
    
    buf = v_buf;
    ehdr = getStructAt<EhdrT>(buf, 0, sizeof(EhdrT), allowExceptionsFromBuffer);

    if (isVariantNullptr(this->ehdr))
        throw ExeException("Could not wrap ELFCore: invalid ELF Header!");

    EhdrT *ehdrPtr = std::get<EhdrT*>(this->ehdr);

    this->phdrs = getStructAt<PhdrT>(buf, ehdrPtr->e_phoff, ehdrPtr->e_phnum, allowExceptionsFromBuffer);
    if (isVariantNullptr(this->phdrs))
        throw ExeException("Could not wrap ELFCore: invalid Program Header!");

    if (ehdrPtr->e_shoff != 0 && ehdrPtr->e_shnum != 0)
        this->shdrs = getStructAt<ShdrT>(buf, ehdrPtr->e_shoff, ehdrPtr->e_shnum, allowExceptionsFromBuffer);
    if (isVariantNullptr(this->shdrs))
        throw ExeException("Could not wrap ELFCore: invalid Section Header!");

    return true;
}

void ELFCore::reset() {
    // Reset buf
    buf = nullptr;

    // Reset the variants to their default state.
    // This is necessary to ensure that the variant does not hold onto 
    // any previous pointers and is ready for a new wrap operation.
    ehdr.emplace<Elf64_Ehdr*>(nullptr);
    phdrs.emplace<Elf64_Phdr*>(nullptr);
    shdrs.emplace<Elf64_Shdr*>(nullptr);

    // Uncache variables
    cachedImageBase = UINT64_MAX;
    cachedImageBaseValid = false;

    cachedImageSize = 0;
    cachedImageSizeValid = false;
}

bool ELFCore::wrap(AbstractByteBuffer *buf) {
    if (!buf) throw ExeException("Could not wrap ELFCore: buffer is null!");

    // buf = dynamic_cast<AbstractByteBuffer*>(v_buf);
    const bool allowExceptionsFromBuffer = false;

    // reset all:
    reset();

    unsigned char e_ident[EI_NIDENT] = {0};

    if (buf->getContentSize() < EI_NIDENT) {
        throw ExeException("Could not wrap ELFCore: buffer too small for ELF Header!");
    }

    std::memcpy(e_ident, buf->getContentAt(0, EI_NIDENT, allowExceptionsFromBuffer), EI_NIDENT);

    if (e_ident[EI_MAG0] != ELFMAG0 || e_ident[EI_MAG1] != ELFMAG1 ||
        e_ident[EI_MAG2] != ELFMAG2 || e_ident[EI_MAG3] != ELFMAG3)
        throw ExeException("Could not wrap ELFCore: not a valid ELF file!");

    return (e_ident[EI_CLASS] == ELFCLASS64)
    ? wrapElfHeaders<Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr>(buf, allowExceptionsFromBuffer)
    : wrapElfHeaders<Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr>(buf, allowExceptionsFromBuffer);
}

// bufsize_t ELFCore::getAlignment() const {
//     return std::visit([](auto phdrsPtr) -> bufsize_t {
//         if (!phdrsPtr) return 0;
//         return phdrsPtr->p_align;  // Often, alignment is uniform across all program headers
//     }, phdrs);
// }

bufsize_t ELFCore::getAlignment() const {
    bufsize_t alignment = 1;

    std::visit([&](auto phdrsPtr) {
        using PhdrT = std::remove_pointer_t<decltype(phdrsPtr)>;
        if (!phdrsPtr) return;

        for (size_t i = 0; i < elfProgramHdrsCount(); ++i) {
            const PhdrT &phdr = phdrsPtr[i];
            if (phdr.p_type != PT_LOAD) continue;

            alignment = std::max(alignment, static_cast<bufsize_t>(phdr.p_align));
        }
    }, getPhdrsVariant());

    return alignment;
}

bufsize_t ELFCore::getImageSize() const {
    if (cachedImageSizeValid) return cachedImageSize;
    
    offset_t maxEnd = 0;
    offset_t minBase = UINT64_MAX;

    std::visit([&](auto phdrsPtr) {
        using PhdrT = std::remove_pointer_t<decltype(phdrsPtr)>;
        if (!phdrsPtr) return;

        for (size_t i = 0; i < this->elfProgramHdrsCount(); ++i) {
            const PhdrT &phdr = phdrsPtr[i];
            if (phdr.p_type != PT_LOAD) continue;

            offset_t start = static_cast<offset_t>(phdr.p_vaddr);
            offset_t   end = static_cast<offset_t>(start + phdr.p_memsz);

            minBase = std::min(minBase, start);
            maxEnd  = std::max(maxEnd, end);
        }
    }, getPhdrsVariant());

    if (minBase == UINT64_MAX || maxEnd == 0) {
        cachedImageSize = 0;
    } else {
        cachedImageSize = maxEnd - minBase;
    }

    cachedImageSizeValid = true;
    return cachedImageSize;
}

offset_t ELFCore::getEntryPoint() const {
    return std::visit([](auto ehdrPtr) -> offset_t {
        if (!ehdrPtr) return INVALID_ADDR;
        return static_cast<offset_t>(ehdrPtr->e_entry);
    }, ehdr);
}

offset_t ELFCore::getImageBase() const  { 
    if (cachedImageBaseValid) return cachedImageBase;

    offset_t base = UINT64_MAX;

    if (elfProgramHdrsCount() == 0) {
        cachedImageBase = 0; // No program headers, assume base is 0
        cachedImageBaseValid = true;
        return cachedImageBase;
    }

    std::visit([&](auto phdrsPtr) {
        using PhdrType = std::remove_pointer_t<decltype(phdrsPtr)>;
        if (!phdrsPtr) return;

        for (size_t i = 0; i < elfProgramHdrsCount(); ++i) {
            const PhdrType &phdr = phdrsPtr[i];

            if (phdr.p_type == PT_LOAD)
                base = std::min(base, static_cast<offset_t>(phdr.p_vaddr));
        }

    }, this->getPhdrsVariant());

    // we cache the image base so we don't have to loop again.
    cachedImageBase = (base == UINT64_MAX) ? 0 : base;
    cachedImageBaseValid = true;
    return cachedImageBase;
}

// bufsize_t ELFCore::getRawSize() const {
//     offset_t maxEnd = 0;

//     std::visit([&](auto phdrsPtr) {
//         using PhdrT = std::remove_pointer_t<decltype(phdrsPtr)>;
//         if (!phdrsPtr) return;

//         for (size_t i = 0; i < elfProgramHdrsCount(); ++i) {
//             const PhdrT &phdr = phdrsPtr[i];
//             if (phdr.p_type != PT_LOAD) continue;

//             offset_t end = static_cast<offset_t>(phdr.p_offset + phdr.p_filesz);
//             maxEnd = std::max(maxEnd, end);
//         }
//     }, getPhdrsVariant());

//     return maxEnd;
// } 

bufsize_t ELFCore::getVirtualSize() const {
    offset_t maxEnd = 0;
    offset_t minBase = UINT64_MAX;

    std::visit([&](auto phdrsPtr) {
        using PhdrT = std::remove_pointer_t<decltype(phdrsPtr)>;
        if (!phdrsPtr) return;

        for (size_t i = 0; i < elfProgramHdrsCount(); ++i) {
            const PhdrT &phdr = phdrsPtr[i];
            if (phdr.p_type != PT_LOAD) continue;

            offset_t start = static_cast<offset_t>(phdr.p_vaddr);
            offset_t end = static_cast<offset_t>(start + phdr.p_memsz);

            minBase = std::min(minBase, start);
            maxEnd = std::max(maxEnd, end);
        }
    }, getPhdrsVariant());

    if (minBase == UINT64_MAX) return 0;
    return maxEnd - minBase;
}

Executable::exe_bits ELFCore::getHdrBitMode() const {
    if (std::holds_alternative<Elf64_Ehdr*>(ehdr)) return Executable::BITS_64;
    return Executable::BITS_32; // Fallback or invalid state
}

Executable::exe_arch ELFCore::getHdrArch() const {
    auto ehdrVar = getEhdrVariant();

    return std::visit([](auto ehdrPtr) -> Executable::exe_arch {
        if (!ehdrPtr) return Executable::ARCH_UNKNOWN;

        switch (ehdrPtr->e_machine) {
            case EM_386:     return Executable::ARCH_INTEL;
            case EM_X86_64:  return Executable::ARCH_INTEL;
            case EM_ARM:     return Executable::ARCH_ARM;
            case EM_AARCH64: return Executable::ARCH_ARM;
            default:         return Executable::ARCH_UNKNOWN;
        }
    }, ehdrVar);
}

offset_t ELFCore::elfProgramHdrsOffset() const {
    return std::visit([](auto ehdrPtr) -> offset_t {
        if (!ehdrPtr) return 0;
        return static_cast<offset_t>(ehdrPtr->e_phoff);
    }, ehdr);
}

offset_t ELFCore::elfSectionHdrsOffset() const {
    return std::visit([](auto ehdrPtr) -> offset_t {
        if (!ehdrPtr) return 0;
        return static_cast<offset_t>(ehdrPtr->e_shoff);
    }, ehdr);
}

offset_t ELFCore::elfProgramHdrsSize() const {
    return std::visit([](auto ehdrPtr) -> offset_t {
        if (!ehdrPtr) return 0;
        return static_cast<offset_t>(ehdrPtr->e_phentsize * ehdrPtr->e_phnum);
    }, ehdr);
}

offset_t ELFCore::elfSectionHdrsSize() const {
    return std::visit([](auto ehdrPtr) -> offset_t {
        if (!ehdrPtr) return 0;
        return static_cast<offset_t>(ehdrPtr->e_shentsize * ehdrPtr->e_shnum);
    }, ehdr);
}

size_t ELFCore::elfProgramHdrsCount() const { 
    return std::visit([](auto ehdrPtr) -> size_t {
        if (!ehdrPtr) return 0;
        return static_cast<size_t>(ehdrPtr->e_phnum);      
    }, ehdr);
}

size_t ELFCore::elfSectionHdrsCount() const {
    return std::visit([](auto ehdrPtr) -> size_t {
        if (!ehdrPtr) return 0;
        return static_cast<size_t>(ehdrPtr->e_shnum);
    }, ehdr);    
}

std::variant<Elf32_Ehdr*, Elf64_Ehdr*> ELFCore::getEhdrVariant() const {
    if (is64()) return getElfHeader<Elf64_Ehdr>();
    return getElfHeader<Elf32_Ehdr>();
}

std::variant<Elf32_Phdr*, Elf64_Phdr*> ELFCore::getPhdrsVariant() const {
    if (is64()) return getProgramHeaders<Elf64_Phdr>();
    return getProgramHeaders<Elf32_Phdr>();
}

std::variant<Elf32_Shdr*, Elf64_Shdr*> ELFCore::getShdrsVariant() const {
    if (is64()) return getSectionHeaders<Elf64_Shdr>();
    return getSectionHeaders<Elf32_Shdr>();
}

template<typename EhdrT>
std::variant<Elf32_Ehdr*, Elf64_Ehdr*> ELFCore::getEhdrVariantT() const {
    return getElfHeader<EhdrT>();
}

template<typename PhdrT>
std::variant<Elf32_Phdr*, Elf64_Phdr*> ELFCore::getPhdrsVariantT() const {
    return getProgramHeaders<PhdrT>();
}

template<typename ShdrT>
std::variant<Elf32_Shdr*, Elf64_Shdr*> ELFCore::getShdrsVariantT() const {
    return getSectionHeaders<ShdrT>();
}

// templated getters for ELF structures
template <typename T>
T* ELFCore::getElfHeader() const {
    // compile-time check to ensure T is either Elf32_Ehdr or Elf64_Ehdr
    static_assert(std::is_same_v<T, Elf32_Ehdr> || std::is_same_v<T, Elf64_Ehdr>,
                  "Invalid type passed to getElfHeader<T>() — must be Elf32_Ehdr or Elf64_Ehdr");
    
    if (std::holds_alternative<T*>(ehdr)) {
        return std::get<T*>(ehdr);
    }
    return nullptr;
}

// templated getters for Program and Section headers
template <typename T>
T* ELFCore::getProgramHeaders() const {
    // compile-time check to ensure T is either Elf32_Phdr or Elf64_Phdr
    static_assert(std::is_same_v<T, Elf32_Phdr> || std::is_same_v<T, Elf64_Phdr>,
                  "Invalid type passed to getProgramHeaders<T>() — must be Elf32_Phdr or Elf64_Phdr");
    
    if (std::holds_alternative<T*>(phdrs)) {
        return std::get<T*>(phdrs);
    }
    return nullptr;
}

template <typename T>
T* ELFCore::getSectionHeaders() const {
    // compile-time check to ensure T is either Elf32_Shdr or Elf64_Shdr
    static_assert(std::is_same_v<T, Elf32_Shdr> || std::is_same_v<T, Elf64_Shdr>,
                  "Invalid type passed to getSectionHeaders<T>() — must be Elf32_Shdr or Elf64_Shdr");
    
    if (std::holds_alternative<T*>(shdrs)) {
        return std::get<T*>(shdrs);
    }
    return nullptr;
}
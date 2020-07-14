#include "iterator.hpp"
#include "aliases.hpp"
#include "traits.hpp"

#include <memory>

namespace elf {
    // Enums representing header field values
    enum elf_class { elf32 = 1, elf64 = 2 };
    enum endianness { little = 1, big = 2 };
    enum os_abi {
        abi_systemv,
        abi_hpux,
        abi_netbsd,
        abi_linux,
        abi_gnuhurd,
        abi_solaris = 0x6,
        abi_aix,
        abi_irix,
        abi_freebsd,
        abi_tru64,
        abi_novell,
        abi_openbsd,
        abi_openvms,
        abi_nskernel,
        abi_aros,
        abi_fenixos,
        abi_cloudabi,
        abi_openvos
    };
    enum obj_type {
        et_none,
        et_rel,
        et_exec,
        et_dyn,
        et_core,
        et_loos = 0xfe00,
        et_hios = 0xfeff,
        et_loproc = 0xff00,
        et_hiproc = 0xffff
    };
    enum target_isa {
        isa_none,
        isa_sparc   = 0x2,
        isa_x86,
        isa_mips    = 0x8,
        isa_ppc32   = 0x14,
        isa_ppc64,
        isa_s390,
        isa_arm     = 0x28,
        isa_superh  = 0x2a,
        isa_ia64    = 0x32,
        isa_amd64   = 0x3e,
        isa_aarch64 = 0xb7,
        isa_riscv   = 0xf3,
        isa_impl    = 0xffff
    };
    enum seg_type {
        pt_null,
        pt_load,
        pt_dynamic,
        pt_interp,
        pt_note,
        pt_shlib,
        pt_phdr,
        pt_tls,
        pt_loos = 0x60000000,
        pt_hios = 0x6fffffff,
        pt_loproc = 0x70000000,
        pt_hiproc = 0x7fffffff,
    };
    enum sect_header_type {
        sht_null,
        sht_progbits,
        sht_symtab,
        sht_strtab,
        sht_rela,
        sht_hash,
        sht_dynamic,
        sht_note,
        sht_nobits,
        sht_rel,
        sht_shlib,
        sht_dynsym,
        sht_init_array,
        sht_fini_array,
        sht_preinit_array,
        sht_group,
        sht_symtab_shndx,
        sht_num,
        sht_loos = 0x60000000,
        sht_hios = 0x6fffffff,
        sht_loproc = 0x70000000,
        sht_hiproc = 0x7fffffff
    };

    enum sect_attributes {
        shf_write               = 0x1,
        shf_alloc,
        shf_execinstr           = 0x4,
        shf_merge               = 0x10,
        shf_strings             = 0x20,
        shf_info_link           = 0x40,
        shf_link_order          = 0x80,
        shf_os_nonconforming    = 0x100,
        shf_group               = 0x200,
        shf_tls                 = 0x400,
        shf_maskos              = 0x0ff00000,
        shf_maskproc            = 0xf0000000,
        shf_ordered             = 0x04000000,
        shf_exclude             = 0x08000000
    };

    // ELF version
    const char g_elf_version = 1;

    // Specialization bases
    template <target tprocw = target::bits64> struct file_header {};
    template <target tprocw = target::bits64> struct program_header {};
    template <target tprocw = target::bits64> struct section_header {};

    // A macro for defining 32 and 64-bit file headers
    #define DEFINE_FSH_STRUCTS(w) \
    template <> struct file_header <target::bits##w> : public traits::iterable { \
        struct e_ident_s { \
            const u8    ei_mag[4]       = { '\x7f', 'E', 'L', 'F' }, \
                        ei_class        = (u8)elf_class::elf##w; \
            u8          ei_data         = (u8)endianness::little; \
            const u8    ei_version      = g_elf_version; \
            u8          ei_osabi        = (u8)os_abi::abi_systemv, \
                        ei_abiversion   = 0, \
                        ei_pad[7]       = { 0 }; \
        } e_ident; \
        \
        u16         e_type      = (u16)obj_type::et_exec, \
                    e_machine   = (u16)target_isa::isa_amd64; \
        u32         e_version   = g_elf_version; \
        u##w        e_entry     = 0; \
        const u##w  e_phoff     = sizeof(file_header); \
        u##w        e_shoff     = 0; \
        u32         e_flags     = 0; \
        const u16   e_ehsize    = sizeof(file_header); \
        u16         e_phentsize = 0, \
                    e_phnum     = 0, \
                    e_shent     = 0, \
                    e_shnum     = 0, \
                    e_shstrndx  = 0; \
    }; \
    \
    template <> struct section_header <target::bits##w> { \
        u32     sh_name         = 0, \
                sh_type         = sect_header_type::sht_null; \
        u##w    sh_flags        = sect_attributes::shf_execinstr, \
                sh_addr         = 0, \
                sh_offset       = 0, \
                sh_size         = 0; \
        u32     sh_link         = 0, \
                sh_info         = 0; \
        u##w    sh_addralign    = 0, \
                sh_entsize      = 0; \
    };

    // Program header
    template <> struct program_header <target::bits32> {
        u32 p_type      = (u32)seg_type::pt_null,
            p_offset    = 0,
            p_vaddr     = 0,
            p_paddr     = 0,
            p_filesz    = 0,
            p_memsz     = 0,
            p_flags     = 0,
            p_align     = 0;
    };

    template <> struct program_header <target::bits64> {
        u32 p_type      = (u32)seg_type::pt_null,
            p_flags;
        u64 p_offset    = 0,
            p_vaddr     = 0,
            p_paddr     = 0,
            p_filesz    = 0,
            p_memsz     = 0,
            p_align     = 0;
    };

    // Define file and section header structs
    DEFINE_FSH_STRUCTS(32)
    DEFINE_FSH_STRUCTS(64)

    template <target tprocw = target::bits64> struct header {
        file_header <tprocw> fhdr;
        program_header <tprocw> phdr;
        section_header <tprocw> shdr;

        iterator<u8> begin() const {
            return iterator(&fhdr.e_ident.ei_mag[0]);
        }

        iterator<u8> end() const {
            if constexpr (tprocw == target::bits32) {
                return iterator((u8*)((&shdr.sh_entsize)+1));
            }
            return iterator((u8*)((&shdr.sh_entsize)+3));
        }
    };
}
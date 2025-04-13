#include "shared/ExecutablePatcher.h"
#include <filesystem>
#include <algorithm>

// Undefine any macros that might conflict with LIEF's enum values
#ifdef DIRECTORY
#undef DIRECTORY
#endif

// Include LIEF headers only in the implementation
#include <LIEF/LIEF.hpp>

namespace shared {

ExecutablePatcher::ExecutablePatcher(const std::string& executablePath) : _executablePath(executablePath) {
    // Parse the binary using LIEF
    _binary = std::unique_ptr<LIEF::Binary>(LIEF::Parser::parse(executablePath));

    if (!_binary) {
        throw std::runtime_error("Failed to parse executable file: " + executablePath);
    }

    // Determine binary type
    if (dynamic_cast<LIEF::PE::Binary*>(_binary.get())) {
        _binaryType = BinaryType::PE;
    } else if (dynamic_cast<LIEF::ELF::Binary*>(_binary.get())) {
        _binaryType = BinaryType::ELF;
    } else if (dynamic_cast<LIEF::MachO::Binary*>(_binary.get())) {
        _binaryType = BinaryType::MACHO;
    } else {
        throw std::runtime_error("Unsupported binary format for: " + executablePath);
    }
}

ExecutablePatcher::~ExecutablePatcher() = default;

void ExecutablePatcher::addResource(const std::string& name, const std::vector<uint8_t>& data) {
    // Validate resource name (only allow alphanumeric and underscore)
    if (name.empty() || !std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; })) {
        throw std::runtime_error("Invalid resource name. Only alphanumeric characters and underscores are allowed.");
    }

    // Call format-specific implementation
    switch (_binaryType) {
        case BinaryType::PE:
            addResourcePE(name, data);
            break;
        case BinaryType::ELF:
            addResourceELF(name, data);
            break;
        case BinaryType::MACHO:
            addResourceMachO(name, data);
            break;
    }
}

std::vector<uint8_t> ExecutablePatcher::getResource(const std::string& name) const {
    // Call format-specific implementation
    switch (_binaryType) {
        case BinaryType::PE:
            return getResourcePE(name);
        case BinaryType::ELF:
            return getResourceELF(name);
        case BinaryType::MACHO:
            return getResourceMachO(name);
        default:
            throw std::runtime_error("Unsupported binary format");
    }
}

void ExecutablePatcher::save() {
    // Create a builder and build the binary
    switch (_binaryType) {
        case BinaryType::PE: {
            auto* pe = getPE();
            LIEF::PE::Builder builder(*pe);
            builder.build();
            builder.write(_executablePath);
            break;
        }
        case BinaryType::ELF: {
            auto* elf = getELF();
            LIEF::ELF::Builder builder(*elf);
            builder.build();
            builder.write(_executablePath);
            break;
        }
        case BinaryType::MACHO: {
            auto* macho = getMachO();
            // MachO's Builder API is different - it's a static method
            macho->write(_executablePath);
            break;
        }
    }
}

// PE Implementation
void ExecutablePatcher::addResourcePE(const std::string& name, const std::vector<uint8_t>& data) {
    auto* pe = getPE();

    if (!pe) {
        throw std::runtime_error("Invalid PE binary");
    }

    // Create a section name
    const std::string sectionName = getSectionName(name);

    // Check if section already exists
    if (pe->get_section(sectionName)) {
        throw std::runtime_error("Resource with name '" + name + "' already exists");
    }

    // Create a new section
    LIEF::PE::Section section(sectionName);

    // Set section content
    section.content(data);

    // Set section characteristics using the proper Section::CHARACTERISTICS enum
    section.characteristics(
        static_cast<uint32_t>(LIEF::PE::Section::CHARACTERISTICS::MEM_READ) |
        static_cast<uint32_t>(LIEF::PE::Section::CHARACTERISTICS::CNT_INITIALIZED_DATA));

    // Add section to PE binary
    pe->add_section(section);
}

std::vector<uint8_t> ExecutablePatcher::getResourcePE(const std::string& name) const {
    auto* pe = getPE();

    if (!pe) {
        throw std::runtime_error("Invalid PE binary");
    }

    // Look for the section with the resource name
    const std::string sectionName = getSectionName(name);
    LIEF::PE::Section* section = pe->get_section(sectionName);

    if (!section) {
        throw std::runtime_error("Resource '" + name + "' not found");
    }

    // Convert LIEF::span to std::vector with braced initializer list
    auto content = section->content();
    return { content.begin(), content.end() };
}

// ELF Implementation
void ExecutablePatcher::addResourceELF(const std::string& name, const std::vector<uint8_t>& data) {
    auto* elf = getELF();

    if (!elf) {
        throw std::runtime_error("Invalid ELF binary");
    }

    // Create a section name
    const std::string sectionName = getSectionName(name);

    // Check if section already exists
    if (elf->get_section(sectionName)) {
        throw std::runtime_error("Resource with name '" + name + "' already exists");
    }

    // Create a new section - LIEF will create this with appropriate defaults
    LIEF::ELF::Section section{ sectionName };

    // Set section content
    section.content(data);

    // Set section flags - Using SHF_ALLOC (0x2)
    section.flags(0x2);  // SHF_ALLOC

    // Add section to ELF binary
    elf->add(section);
}

std::vector<uint8_t> ExecutablePatcher::getResourceELF(const std::string& name) const {
    auto* elf = getELF();

    if (!elf) {
        throw std::runtime_error("Invalid ELF binary");
    }

    // Look for the section with the resource name
    const std::string sectionName = getSectionName(name);
    LIEF::ELF::Section* section = elf->get_section(sectionName);

    if (!section) {
        throw std::runtime_error("Resource '" + name + "' not found");
    }

    // Convert LIEF::span to std::vector with braced initializer list
    auto content = section->content();
    return { content.begin(), content.end() };
}

// MachO Implementation
void ExecutablePatcher::addResourceMachO(const std::string& name, const std::vector<uint8_t>& data) {
    auto* macho = getMachO();

    if (!macho) {
        throw std::runtime_error("Invalid MachO binary");
    }

    // Create a section name (limited to 16 chars in MachO)
    std::string sectionName = getSectionName(name);
    if (sectionName.length() > 16) {
        sectionName = sectionName.substr(0, 16);
    }

    // Check if section already exists
    for (const auto& section : macho->sections()) {
        if (section.name() == sectionName) {
            throw std::runtime_error("Resource with name '" + name + "' already exists");
        }
    }

    // Find a suitable segment (typically __DATA)
    LIEF::MachO::SegmentCommand* dataSegment = nullptr;
    for (auto& segment : macho->segments()) {
        if (segment.name() == "__DATA") {
            dataSegment = &segment;
            break;
        }
    }

    if (!dataSegment) {
        throw std::runtime_error("Could not find __DATA segment in MachO binary");
    }

    // For MachO, we'll create a segment directly in the proper segment
    // Create a new section in the binary with the correct API
    LIEF::MachO::Section section;
    section.name(sectionName);
    section.segment_name("__DATA");

    // Set section content
    section.content(data);

    // Set type and flags for MachO
    // S_REGULAR = 0x0
    section.type(static_cast<LIEF::MachO::Section::TYPE>(0x0));

    // S_ATTR_SOME_INSTRUCTIONS = 0x400, S_ATTR_PURE_INSTRUCTIONS = 0x80000000
    const uint32_t sectionFlags = 0x400 | 0x80000000;
    section.flags(sectionFlags);

    // Add section to the binary
    macho->add_section(*dataSegment, section);
}

std::vector<uint8_t> ExecutablePatcher::getResourceMachO(const std::string& name) const {
    auto* macho = getMachO();

    if (!macho) {
        throw std::runtime_error("Invalid MachO binary");
    }

    // Create section name (limited to 16 chars in MachO)
    std::string sectionName = getSectionName(name);
    if (sectionName.length() > 16) {
        sectionName = sectionName.substr(0, 16);
    }

    // Look for the section with the resource name
    for (const auto& section : macho->sections()) {
        if (section.name() == sectionName) {
            // Convert LIEF::span to std::vector with braced initializer list
            auto content = section.content();
            return { content.begin(), content.end() };
        }
    }

    throw std::runtime_error("Resource '" + name + "' not found");
}

// Utility methods
std::string ExecutablePatcher::getSectionName(const std::string& resourceName) {
    return std::string(kResourcePrefix) + resourceName;
}

LIEF::PE::Binary* ExecutablePatcher::getPE() const {
    return dynamic_cast<LIEF::PE::Binary*>(_binary.get());
}

LIEF::ELF::Binary* ExecutablePatcher::getELF() const {
    return dynamic_cast<LIEF::ELF::Binary*>(_binary.get());
}

LIEF::MachO::Binary* ExecutablePatcher::getMachO() const {
    return dynamic_cast<LIEF::MachO::Binary*>(_binary.get());
}

}  // namespace shared

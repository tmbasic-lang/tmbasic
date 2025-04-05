#pragma once

#include "../common.h"
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

// Forward declare LIEF types instead of including headers
namespace LIEF {
class Binary;

namespace PE {
class Binary;
}

namespace ELF {
class Binary;
}

namespace MachO {
class Binary;
}
}  // namespace LIEF

namespace shared {

/**
 * ExecutablePatcher allows embedding binary resources into existing executable files.
 * It supports Windows PE, macOS Mach-O, and Linux ELF formats with a unified interface.
 * Resources are added as proper sections to maintain binary format validity.
 */
class ExecutablePatcher {
   public:
    static constexpr const char* kResourcePcode = "PCODE";
    static constexpr const char* kResourceTzdb = "TZDB";

    /**
     * Constructor
     * @param executablePath Path to the executable file to patch
     * @throws std::runtime_error if the file doesn't exist or isn't a valid executable
     */
    explicit ExecutablePatcher(const std::string& executablePath);

    /**
     * Destructor must be provided because LIEF::Binary is incomplete.
     */
    ~ExecutablePatcher();

    /**
     * Adds a binary resource to the executable
     * @param name Unique name for the resource (used as section name with prefix)
     * @param data The binary data to embed
     * @throws std::runtime_error if operation fails
     */
    void addResource(const std::string& name, const std::vector<uint8_t>& data);

    /**
     * Retrieves a binary resource from the executable
     * @param name The unique name of the resource to retrieve
     * @return The binary data of the resource
     * @throws std::runtime_error if the resource doesn't exist
     */
    std::vector<uint8_t> getResource(const std::string& name) const;

    /**
     * Saves the changes to the executable file
     * @throws std::runtime_error if the save operation fails
     */
    void save();

   private:
    // Constants for section naming
    static constexpr const char* kResourcePrefix = "TM_";

    // Path to the executable file
    std::string _executablePath;

    // Parsed binary (using std::unique_ptr with LIEF::Binary*)
    std::unique_ptr<LIEF::Binary> _binary;

    // Format type of the binary
    enum class BinaryType { PE, ELF, MACHO } _binaryType;

    // Internal methods for type-specific implementations
    void addResourcePE(const std::string& name, const std::vector<uint8_t>& data);
    void addResourceELF(const std::string& name, const std::vector<uint8_t>& data);
    void addResourceMachO(const std::string& name, const std::vector<uint8_t>& data);

    std::vector<uint8_t> getResourcePE(const std::string& name) const;
    std::vector<uint8_t> getResourceELF(const std::string& name) const;
    std::vector<uint8_t> getResourceMachO(const std::string& name) const;

    // Utility methods
    static std::string getSectionName(const std::string& resourceName);
    LIEF::PE::Binary* getPE() const;
    LIEF::ELF::Binary* getELF() const;
    LIEF::MachO::Binary* getMachO() const;
};

}  // namespace shared

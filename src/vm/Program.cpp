#include "vm/Program.h"
#include "shared/serializedProgramConstants.h"
#include "vm/List.h"
#include "vm/Map.h"
#include "vm/Record.h"
#include "vm/String.h"

namespace vm {

// See ProgramWriter in compiler/CompiledProgram.cpp.
class ProgramReader {
   public:
    const std::vector<uint8_t>& vec;
    size_t offset{ 0 };

    explicit ProgramReader(const std::vector<uint8_t>& vec) : vec(vec) {}

    bool eof() { return offset >= vec.size(); }

    template <typename TInt>
    TInt readInt() {
        TInt x{};
        memcpy(&x, &vec.at(offset), sizeof(TInt));
        offset += sizeof(TInt);
        return x;
    }

    decimal::Decimal readDecimal() {
        mpd_uint128_triple_t triple{};
        triple.sign = readInt<uint8_t>();
        triple.hi = readInt<uint64_t>();
        triple.lo = readInt<uint64_t>();
        triple.exp = readInt<int64_t>();
        return decimal::Decimal{ triple };
    }

    boost::intrusive_ptr<vm::String> readString() {
        size_t const length = readInt<uint32_t>();
        std::string_view const sv{ reinterpret_cast<const char*>(&vec.at(offset)), length };
        std::string const str{ sv };
        offset += length;
        return boost::make_intrusive_ptr<vm::String>(str);
    }

    boost::intrusive_ptr<vm::ValueList> readValueList() {
        vm::ValueListBuilder vlb{};
        auto count = readInt<uint32_t>();
        for (uint32_t i = 0; i < count; i++) {
            vlb.items.push_back(vm::Value{ readDecimal() });
        }
        return boost::make_intrusive_ptr<vm::ValueList>(&vlb);
    }

    boost::intrusive_ptr<vm::Object> readObject() {
        auto type = static_cast<ObjectType>(readInt<uint8_t>());
        switch (type) {
            case ObjectType::kString:
                return readString();
            default:
                throw std::runtime_error("Unsupported object type for serialization.");
        }
    }
};

void Program::deserialize(const std::vector<uint8_t>& pcode) {
    ProgramReader reader{ pcode };
    startupProcedureIndex = reader.readInt<uint32_t>();

    while (!reader.eof()) {
        switch (reader.readInt<uint8_t>()) {
            case shared::kEofByte:
                return;

            case shared::kProcedureByte: {
                auto numBytes = reader.readInt<uint32_t>();
                auto procedure = std::make_unique<vm::Procedure>();
                procedure->instructions.insert(
                    procedure->instructions.end(), reader.vec.begin() + static_cast<ptrdiff_t>(reader.offset),
                    reader.vec.begin() + static_cast<ptrdiff_t>(reader.offset + numBytes));
                reader.offset += numBytes;
                procedures.push_back(std::move(procedure));
                break;
            }

            case shared::kValueByte:
                globalValues.emplace_back(reader.readDecimal());
                break;

            case shared::kObjectByte:
                globalObjects.push_back(reader.readObject());
                break;

            default:
                throw std::runtime_error("Unexpected byte in the pcode.");
        }
    }
}

}  // namespace vm

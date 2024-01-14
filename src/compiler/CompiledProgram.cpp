#include "compiler/CompiledProgram.h"
#include "shared/serializedProgramConstants.h"

namespace compiler {

// See ProgramReader in vm/Program.cpp.
class ProgramWriter {
   public:
    std::vector<uint8_t> vec;

    template <typename TInt>
    void writeInt(TInt value) {
        std::array<uint8_t, sizeof(TInt)> bytes{};
        memcpy(bytes.data(), &value, sizeof(TInt));
        vec.insert(vec.end(), bytes.begin(), bytes.end());
    }

    void writeDecimal(const decimal::Decimal& dec) {
        auto triple = dec.as_uint128_triple();
        writeInt<uint8_t>(triple.sign);
        writeInt<uint64_t>(triple.hi);
        writeInt<uint64_t>(triple.lo);
        writeInt<int64_t>(triple.exp);
    }

    void writeString(const std::string& utf8) {
        writeInt<uint32_t>(static_cast<uint32_t>(utf8.size()));
        vec.insert(vec.end(), utf8.begin(), utf8.end());
    }
};

std::vector<uint8_t> CompiledProgram::serialize() const {
    ProgramWriter writer{};
    writer.writeInt<uint32_t>(static_cast<uint32_t>(vmStartupProcedureIndex));

    for (const auto& p : vmProcedures) {
        writer.writeInt<uint8_t>(static_cast<uint8_t>(shared::kProcedureByte));
        writer.writeInt<uint32_t>(static_cast<uint32_t>(p.size()));
        writer.vec.insert(writer.vec.end(), p.begin(), p.end());
    }

    for (const auto& v : vmGlobalValues) {
        writer.writeInt<uint8_t>(static_cast<uint8_t>(shared::kValueByte));
        writer.writeDecimal(v);
    }

    for (const auto& p : vmGlobalObjects) {
        const auto& objectType = p.first;
        const auto& initialString = p.second;
        writer.writeInt<uint8_t>(static_cast<uint8_t>(shared::kObjectByte));
        writer.writeInt<uint8_t>(static_cast<uint8_t>(objectType));
        if (objectType == shared::ObjectType::kString) {
            writer.writeString(initialString);
        }
    }

    writer.writeInt<uint8_t>(static_cast<uint8_t>(shared::kEofByte));

    return writer.vec;
}

}  // namespace compiler

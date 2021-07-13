#include "Program.h"
#include "List.h"
#include "Map.h"
#include "Record.h"
#include "String.h"

namespace vm {

constexpr uint8_t kEofByte = 255;
constexpr uint8_t kProcedureByte = 1;
constexpr uint8_t kValueByte = 2;
constexpr uint8_t kObjectByte = 3;

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

    void writeString(const vm::String& obj) {
        const auto& str = dynamic_cast<const vm::String&>(obj);
        auto utf8 = str.toUtf8();
        writeInt<uint32_t>(static_cast<uint32_t>(utf8.size()));
        vec.insert(vec.end(), utf8.begin(), utf8.end());
    }

    void writeObject(const vm::Object& obj) {
        writeInt<uint8_t>(static_cast<uint8_t>(obj.getObjectType()));
        switch (obj.getObjectType()) {
            case ObjectType::kString:
                writeString(dynamic_cast<const vm::String&>(obj));
                break;
            default:
                throw std::runtime_error("Unsupported object type for serialization.");
        }
    }
};

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

    boost::local_shared_ptr<vm::String> readString() {
        size_t length = readInt<uint32_t>();
        std::string_view sv{ reinterpret_cast<const char*>(&vec.at(offset)), length };
        std::string str{ sv };
        offset += length;
        return boost::make_local_shared<vm::String>(str);
    }

    boost::local_shared_ptr<vm::ValueList> readValueList() {
        vm::ValueListBuilder vlb{};
        auto count = readInt<uint32_t>();
        for (uint32_t i = 0; i < count; i++) {
            vlb.items.push_back(vm::Value{ readDecimal() });
        }
        return boost::make_local_shared<vm::ValueList>(&vlb);
    }

    boost::local_shared_ptr<vm::Object> readObject() {
        switch (static_cast<ObjectType>(readInt<uint8_t>())) {
            case ObjectType::kString:
                return readString();
            default:
                throw std::runtime_error("Unsupported object type for serialization.");
        }
    }
};

std::vector<uint8_t> Program::serialize() {
    ProgramWriter writer{};
    writer.writeInt<uint32_t>(static_cast<uint32_t>(startupProcedureIndex));

    for (const auto& p : procedures) {
        writer.writeInt<uint8_t>(static_cast<uint8_t>(kProcedureByte));
        writer.writeInt<uint32_t>(static_cast<uint32_t>(p->instructions.size()));
        writer.vec.insert(writer.vec.end(), p->instructions.begin(), p->instructions.end());
    }

    for (const auto& v : globalValues) {
        writer.writeInt<uint8_t>(static_cast<uint8_t>(kValueByte));
        writer.writeDecimal(v.num);
    }

    for (const auto& o : globalObjects) {
        writer.writeInt<uint8_t>(static_cast<uint8_t>(kObjectByte));
        writer.writeObject(*o);
    }

    writer.writeInt<uint8_t>(static_cast<uint8_t>(kEofByte));

    return writer.vec;
}

void Program::deserialize(const std::vector<uint8_t>& pcode) {
    ProgramReader reader{ pcode };
    startupProcedureIndex = reader.readInt<uint32_t>();

    while (!reader.eof()) {
        switch (reader.readInt<uint8_t>()) {
            case kEofByte:
                return;

            case kProcedureByte: {
                auto numBytes = reader.readInt<uint32_t>();
                auto procedure = std::make_unique<vm::Procedure>();
                procedure->instructions.insert(
                    procedure->instructions.end(), reader.vec.begin() + reader.offset,
                    reader.vec.begin() + reader.offset + numBytes);
                reader.offset += numBytes;
                procedures.push_back(std::move(procedure));
                break;
            }
            case kValueByte:
                globalValues.emplace_back(reader.readDecimal());
                break;
            case kObjectByte:
                globalObjects.push_back(reader.readObject());
                break;
            default:
                throw std::runtime_error("Unexpected byte in the pcode.");
        }
    }
}

}  // namespace vm

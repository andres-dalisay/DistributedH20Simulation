#include <cereal/types/string.hpp>

struct Log {
    std::string id;
    std::string type;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(id, type);
    }
};

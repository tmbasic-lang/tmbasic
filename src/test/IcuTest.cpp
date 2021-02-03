#include "../common.h"
#include "../shared/util/UTextPtr.h"
#include "gtest/gtest.h"

using icu::BreakIterator;
using icu::Locale;
using util::UTextPtr;

TEST(IcuTest, GraphemeClusters1) {
    auto status = U_ZERO_ERROR;
    auto utext = UTextPtr("née");
    auto iter = std::unique_ptr<BreakIterator>(BreakIterator::createCharacterInstance(Locale::getUS(), status));
    iter->setText(utext.get(), status);
    auto index = iter->first();
    std::vector<int32_t> indices;
    while (index != BreakIterator::DONE) {
        indices.push_back(index);
        index = iter->next();
    }

    ASSERT_EQ(4, indices.size());
    ASSERT_EQ(0, indices.at(0));
    ASSERT_EQ(1, indices.at(1));
    ASSERT_EQ(3, indices.at(2));
    ASSERT_EQ(4, indices.at(3));
}

TEST(IcuTest, GraphemeClusters2) {
    auto status = U_ZERO_ERROR;
    const auto* str = "ᄀᅶ섀ᇧ";  // composed using separate jamo code points, not precomposed syllables
    ASSERT_EQ(15, strlen(str));      // double check that it's separate jamo code points
    auto utext = UTextPtr(str);
    auto iter = std::unique_ptr<BreakIterator>(BreakIterator::createCharacterInstance(Locale::getKorean(), status));
    iter->setText(utext.get(), status);
    auto index = iter->first();
    std::vector<int32_t> indices;
    while (index != BreakIterator::DONE) {
        indices.push_back(index);
        index = iter->next();
    }

    ASSERT_EQ(3, indices.size());
    ASSERT_EQ(0, indices.at(0));
    ASSERT_EQ(6, indices.at(1));
    ASSERT_EQ(15, indices.at(2));
}

TEST(IcuTest, AvailableLocales) {
    int32_t count = 0;
    auto* locales = Locale::getAvailableLocales(count);
    auto en_US = false;
    auto fr_FR = false;
    auto ja_JP = false;

    for (int32_t i = 0; i < count; i++) {
        en_US |= std::string(locales[i].getName()) == "en_US";
        fr_FR |= std::string(locales[i].getName()) == "fr_FR";
        ja_JP |= std::string(locales[i].getName()) == "ja_JP";
    }

    ASSERT_TRUE(en_US);
    ASSERT_TRUE(fr_FR);
    ASSERT_TRUE(ja_JP);
}

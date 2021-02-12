#include "../common.h"
#include "gtest/gtest.h"

using icu::BreakIterator;
using icu::Collator;
using icu::Locale;
using icu::StringEnumeration;
using icu::TimeZone;
using icu::UnicodeString;

TEST(IcuTest, GraphemeClusters1) {
    auto status = U_ZERO_ERROR;
    // e + COMBINING ACUTE ACCENT
    auto ustr = UnicodeString::fromUTF8(
        "n"
        "e\xCC\x81"
        "e");
    auto iter = std::unique_ptr<BreakIterator>(BreakIterator::createCharacterInstance(Locale::getUS(), status));
    iter->setText(ustr);
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
    auto ustr = UnicodeString::fromUTF8(str);
    auto iter = std::unique_ptr<BreakIterator>(BreakIterator::createCharacterInstance(Locale::getKorean(), status));
    iter->setText(ustr);
    auto index = iter->first();
    std::vector<int32_t> indices;
    while (index != BreakIterator::DONE) {
        indices.push_back(index);
        index = iter->next();
    }

    ASSERT_EQ(3, indices.size());
    ASSERT_EQ(0, indices.at(0));
    ASSERT_EQ(2, indices.at(1));
    ASSERT_EQ(5, indices.at(2));
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

TEST(IcuTest, StringComparison) {
    auto str1 = UnicodeString::fromUTF8("\xC3\xA9");   // LATIN SMALL LETTER E WITH ACUTE
    auto str2 = UnicodeString::fromUTF8("e\xCC\x81");  // e + COMBINING ACUTE ACCENT

    // verify that bitwise comparison fails to see that they are equal, and that we need ICU to do this
    ASSERT_FALSE(str1 == str2);

    auto status = U_ZERO_ERROR;
    auto collator = std::unique_ptr<Collator>(Collator::createInstance(Locale::getUS(), status));
    ASSERT_EQ(UCOL_EQUAL, collator->compare(str1, str2));
}

TEST(IcuTest, Words1) {
    auto status = U_ZERO_ERROR;
    const auto* str = "a dog.  the\tcat-dog?";
    auto ustr = UnicodeString::fromUTF8(str);
    auto iter = std::unique_ptr<BreakIterator>(BreakIterator::createWordInstance(Locale::getUS(), status));
    iter->setText(ustr);
    auto index = iter->first();
    std::vector<int32_t> indices;
    while (index != BreakIterator::DONE) {
        indices.push_back(index);
        index = iter->next();
    }

    // 0
    // 1 "a"
    // 2 " "
    // 5 "dog"
    // 6 "."
    // 8 "  "
    // 11 "the"
    // 12 "\t"
    // 15 "cat"
    // 16 "-"
    // 19 "dog"
    // 20 "?"

    ASSERT_EQ(12, indices.size());
    ASSERT_EQ(0, indices.at(0));
    ASSERT_EQ(1, indices.at(1));
    ASSERT_EQ(2, indices.at(2));
    ASSERT_EQ(5, indices.at(3));
    ASSERT_EQ(6, indices.at(4));
    ASSERT_EQ(8, indices.at(5));
    ASSERT_EQ(11, indices.at(6));
    ASSERT_EQ(12, indices.at(7));
    ASSERT_EQ(15, indices.at(8));
    ASSERT_EQ(16, indices.at(9));
    ASSERT_EQ(19, indices.at(10));
    ASSERT_EQ(20, indices.at(11));
}

TEST(IcuTest, Words2) {
    auto status = U_ZERO_ERROR;
    const auto* str = "これはテストです";
    auto ustr = UnicodeString::fromUTF8(str);
    auto iter = std::unique_ptr<BreakIterator>(BreakIterator::createWordInstance(Locale::getJapan(), status));
    iter->setText(ustr);
    auto index = iter->first();
    std::vector<int32_t> indices;
    while (index != BreakIterator::DONE) {
        indices.push_back(index);
        index = iter->next();
    }

    // 0
    // 2 "これ"
    // 3 "は"
    // 6 "テスト"
    // 8 "です"

    ASSERT_EQ(5, indices.size());
    ASSERT_EQ(0, indices.at(0));
    ASSERT_EQ(2, indices.at(1));
    ASSERT_EQ(3, indices.at(2));
    ASSERT_EQ(6, indices.at(3));
    ASSERT_EQ(8, indices.at(4));
}

TEST(IcuTest, AvailableTimeZones) {
    auto iter = std::unique_ptr<StringEnumeration>(TimeZone::createEnumeration());
    const char* item = nullptr;
    auto status = U_ZERO_ERROR;
    auto americaChicago = false;
    auto cst = false;
    auto etcGmtMinus4 = false;
    auto usCentral = false;
    auto utc = false;

    while ((item = iter->next(nullptr, status)) != nullptr) {
        americaChicago |= strcmp(item, "America/Chicago") == 0;
        cst |= strcmp(item, "CST") == 0;
        etcGmtMinus4 |= strcmp(item, "Etc/GMT-4") == 0;
        usCentral |= strcmp(item, "US/Central") == 0;
        utc |= strcmp(item, "UTC") == 0;
    }

    ASSERT_TRUE(americaChicago);
    ASSERT_TRUE(cst);
    ASSERT_TRUE(etcGmtMinus4);
    ASSERT_TRUE(usCentral);
    ASSERT_TRUE(utc);
}

TEST(IcuTest, GetLocaleByName_Success) {
    auto locale = Locale("en_US");
    ASSERT_EQ("en_US", std::string(locale.getName()));
}

TEST(IcuTest, GetLocaleByName_Failure) {
    auto locale = Locale("asdf");
    ASSERT_EQ("", std::string(locale.getLanguage()));
}

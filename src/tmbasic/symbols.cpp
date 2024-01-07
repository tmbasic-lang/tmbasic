#include "symbols.h"

namespace tmbasic {

// this is intended to be a list of characters that are universally supported.
// this is mostly constrained by Windows conhost and Linux xterm.
// manually sorted: 1.) punctuation, 2.) super/subscripts, 3.) letters
static const std::vector<std::string> _symbols{
    "┌",
    "┬",
    "┐",
    "╒",
    "╤",
    "╕",
    "╓",
    "╥",
    "╖",
    "╔",
    "╦",
    "╗",
    "─",
    "│",
    "═",
    "║",
    "⓪",
    "①",
    "②",
    "③",
    "④",
    "⑤",
    "⑥",
    "⑦",
    "⑧",
    "⑨",
    "⑩",
    "⑪",
    "⑫",
    "⑬",
    "⑭",
    "⑮",
    "├",
    "┼",
    "┤",
    "╞",
    "╪",
    "╡",
    "╟",
    "╫",
    "╢",
    "╠",
    "╬",
    "╣",
    "░",
    "▒",
    "▓",
    "█",
    "⑯",
    "⑰",
    "⑱",
    "⑲",
    "⑳",
    "❶",
    "❷",
    "❸",
    "❹",
    "❺",
    "❻",
    "❼",
    "❽",
    "❾",
    "❿",
    "≤",
    "└",
    "┴",
    "┘",
    "╘",
    "╧",
    "╛",
    "╙",
    "╨",
    "╜",
    "╚",
    "╩",
    "╝",
    "▀",
    "▄",
    "▌",
    "▐",
    "½",
    "⅓",
    "⅔",
    "¼",
    "¾",
    "⅕",
    "⅖",
    "⅗",
    "⅘",
    "⅙",
    "⅚",
    "⅛",
    "⅜",
    "⅝",
    "⅞",
    "≥",
    "ⁱ",
    "⁰",
    "¹",
    "²",
    "³",
    "⁴",
    "⁵",
    "⁶",
    "⁷",
    "⁸",
    "⁹",
    "⁺",
    "⁻",
    "⁼",
    "⁽",
    "⁾",
    "ⁿ",
    "₀",
    "₁",
    "₂",
    "₃",
    "₄",
    "₅",
    "₆",
    "₇",
    "₈",
    "₉",
    "₊",
    "₋",
    "₌",
    "₍",
    "₎",
    "⌠",
    "¡",
    "ǃ",
    "←",
    "↑",
    "→",
    "↓",
    "↔",
    "↕",
    "↨",
    "¢",
    "£",
    "¤",
    "¥",
    "¦",
    "§",
    "¨",
    "ª",
    "«",
    "»",
    "‹",
    "›",
    "¬",
    "¯",
    "°",
    "±",
    "´",
    "µ",
    "¶",
    "·",
    "¸",
    "º",
    "⌡",
    "¿",
    "×",
    "÷",
    "ǀ",
    "ǁ",
    "ǂ",
    "˞",
    ";",
    "‐",
    "‒",
    "–",
    "—",
    "―",
    "‖",
    "†",
    "‡",
    "…",
    "‰",
    "‼",
    "‽",
    "⁄",
    "₣",
    "₤",
    "₦",
    "₧",
    "₨",
    "₩",
    "€",
    "₯",
    "℅",
    "ℓ",
    "№",
    "℗",
    "™",
    "©",
    "®",
    "Ω",
    "℮",
    "Ⅎ",
    "Ↄ",
    "∂",
    "∆",
    "∏",
    "∑",
    "−",
    "∕",
    "√",
    "∞",
    "∟",
    "∩",
    "∫",
    "≈",
    "≠",
    "≡",
    "⌂",
    "⌐",
    "■",
    "□",
    "▪",
    "▫",
    "▬",
    "▲",
    "▴",
    "▸",
    "►",
    "▼",
    "▾",
    "◂",
    "◄",
    "◊",
    "○",
    "◌",
    "●",
    "◘",
    "◙",
    "◦",
    "☺",
    "☻",
    "☼",
    "♀",
    "♂",
    "♠",
    "♣",
    "♥",
    "♦",
    "♪",
    "♫",
    "✶",
    "•",
    "ʻ",
    "ʽ",
    "ʼ",
    "ʿ",
    "ʾ",
    "‘",
    "’",
    "‚",
    "‛",
    "“",
    "‟",
    "”",
    "„",
    "′",
    "″",
    "‴",
    "ˀ",
    "ˁ",
    "˂",
    "˃",
    "˄",
    "˅",
    "ˆ",
    "ˇ",
    "ˈ",
    "ˉ",
    "ˊ",
    "ˋ",
    "ˌ",
    "ˍ",
    "ˎ",
    "ˏ",
    "ː",
    "ˑ",
    "˒",
    "˓",
    "˔",
    "˕",
    "˖",
    "˗",
    "˘",
    "˙",
    "˚",
    "˛",
    "˜",
    "˝",
    "˟",
    "ˠ",
    "ˡ",
    "ˢ",
    "ˣ",
    "ˤ",
    "˪",
    "˫",
    "ˬ",
    "˭",
    "ˮ",
    "ʹ",
    "͵",
    "ͺ",
    "΄",
    "΅",
    "·",
    "᾽",
    "ι",
    "᾿",
    "῀",
    "῁",
    "῍",
    "῎",
    "῏",
    "῝",
    "῞",
    "῟",
    "῭",
    "΅",
    "`",
    "´",
    "῾",
    "‗",
    "‾",
    "⁃",
    "ﬁ",
    "ﬂ",
    "À",
    "Á",
    "Â",
    "Ã",
    "Ä",
    "Å",
    "Æ",
    "Ç",
    "È",
    "É",
    "Ê",
    "Ë",
    "Ì",
    "Í",
    "Î",
    "Ï",
    "Ð",
    "Ñ",
    "Ò",
    "Ó",
    "Ô",
    "Õ",
    "Ö",
    "Ø",
    "Ù",
    "Ú",
    "Û",
    "Ü",
    "Ý",
    "Þ",
    "ß",
    "à",
    "á",
    "â",
    "ã",
    "ä",
    "å",
    "æ",
    "ç",
    "è",
    "é",
    "ê",
    "ë",
    "ì",
    "í",
    "î",
    "ï",
    "ð",
    "ñ",
    "ò",
    "ó",
    "ô",
    "õ",
    "ö",
    "ø",
    "ù",
    "ú",
    "û",
    "ü",
    "ý",
    "þ",
    "ÿ",
    "Ā",
    "ā",
    "Ă",
    "ă",
    "Ą",
    "ą",
    "Ć",
    "ć",
    "Ĉ",
    "ĉ",
    "Ċ",
    "ċ",
    "Č",
    "č",
    "Ď",
    "ď",
    "Đ",
    "đ",
    "Ē",
    "ē",
    "Ĕ",
    "ĕ",
    "Ė",
    "ė",
    "Ę",
    "ę",
    "Ě",
    "ě",
    "Ĝ",
    "ĝ",
    "Ğ",
    "ğ",
    "Ġ",
    "ġ",
    "Ģ",
    "ģ",
    "Ĥ",
    "ĥ",
    "Ħ",
    "ħ",
    "Ĩ",
    "ĩ",
    "Ī",
    "ī",
    "Ĭ",
    "ĭ",
    "Į",
    "į",
    "İ",
    "ı",
    "Ĳ",
    "ĳ",
    "Ĵ",
    "ĵ",
    "Ķ",
    "ķ",
    "ĸ",
    "Ĺ",
    "ĺ",
    "Ļ",
    "ļ",
    "Ľ",
    "ľ",
    "Ŀ",
    "ŀ",
    "Ł",
    "ł",
    "Ń",
    "ń",
    "Ņ",
    "ņ",
    "Ň",
    "ň",
    "ŉ",
    "Ŋ",
    "ŋ",
    "Ō",
    "ō",
    "Ŏ",
    "ŏ",
    "Ő",
    "ő",
    "Œ",
    "œ",
    "Ŕ",
    "ŕ",
    "Ŗ",
    "ŗ",
    "Ř",
    "ř",
    "Ś",
    "ś",
    "Ŝ",
    "ŝ",
    "Ş",
    "ş",
    "Š",
    "š",
    "Ţ",
    "ţ",
    "Ť",
    "ť",
    "Ŧ",
    "ŧ",
    "Ũ",
    "ũ",
    "Ū",
    "ū",
    "Ŭ",
    "ŭ",
    "Ů",
    "ů",
    "Ű",
    "ű",
    "Ų",
    "ų",
    "Ŵ",
    "ŵ",
    "Ŷ",
    "ŷ",
    "Ÿ",
    "Ź",
    "ź",
    "Ż",
    "ż",
    "Ž",
    "ž",
    "ſ",
    "ƀ",
    "Ɓ",
    "Ƃ",
    "ƃ",
    "Ƅ",
    "ƅ",
    "Ɔ",
    "Ƈ",
    "ƈ",
    "Ɖ",
    "Ɗ",
    "Ƌ",
    "ƌ",
    "ƍ",
    "Ǝ",
    "Ə",
    "Ɛ",
    "Ƒ",
    "ƒ",
    "Ɠ",
    "Ɣ",
    "ƕ",
    "Ɩ",
    "Ɨ",
    "Ƙ",
    "ƙ",
    "ƚ",
    "ƛ",
    "Ɯ",
    "Ɲ",
    "ƞ",
    "Ɵ",
    "Ơ",
    "ơ",
    "Ƣ",
    "ƣ",
    "Ƥ",
    "ƥ",
    "Ʀ",
    "Ƨ",
    "ƨ",
    "Ʃ",
    "ƪ",
    "ƫ",
    "Ƭ",
    "ƭ",
    "Ʈ",
    "Ư",
    "ư",
    "Ʊ",
    "Ʋ",
    "Ƴ",
    "ƴ",
    "Ƶ",
    "ƶ",
    "Ʒ",
    "Ƹ",
    "ƹ",
    "ƺ",
    "ƻ",
    "Ƽ",
    "ƽ",
    "ƾ",
    "ƿ",
    "Ǆ",
    "ǅ",
    "ǆ",
    "Ǉ",
    "ǈ",
    "ǉ",
    "Ǌ",
    "ǋ",
    "ǌ",
    "Ǎ",
    "ǎ",
    "Ǐ",
    "ǐ",
    "Ǒ",
    "ǒ",
    "Ǔ",
    "ǔ",
    "Ǖ",
    "ǖ",
    "Ǘ",
    "ǘ",
    "Ǚ",
    "ǚ",
    "Ǜ",
    "ǜ",
    "ǝ",
    "Ǟ",
    "ǟ",
    "Ǡ",
    "ǡ",
    "Ǣ",
    "ǣ",
    "Ǥ",
    "ǥ",
    "Ǧ",
    "ǧ",
    "Ǩ",
    "ǩ",
    "Ǫ",
    "ǫ",
    "Ǭ",
    "ǭ",
    "Ǯ",
    "ǯ",
    "ǰ",
    "Ǳ",
    "ǲ",
    "ǳ",
    "Ǵ",
    "ǵ",
    "Ƕ",
    "Ƿ",
    "Ǹ",
    "ǹ",
    "Ǻ",
    "ǻ",
    "Ǽ",
    "ǽ",
    "Ǿ",
    "ǿ",
    "Ȁ",
    "ȁ",
    "Ȃ",
    "ȃ",
    "Ȅ",
    "ȅ",
    "Ȇ",
    "ȇ",
    "Ȉ",
    "ȉ",
    "Ȋ",
    "ȋ",
    "Ȍ",
    "ȍ",
    "Ȏ",
    "ȏ",
    "Ȑ",
    "ȑ",
    "Ȓ",
    "ȓ",
    "Ȕ",
    "ȕ",
    "Ȗ",
    "ȗ",
    "Ș",
    "ș",
    "Ț",
    "ț",
    "Ȝ",
    "ȝ",
    "Ȟ",
    "ȟ",
    "Ƞ",
    "ȡ",
    "Ȣ",
    "ȣ",
    "Ȥ",
    "ȥ",
    "Ȧ",
    "ȧ",
    "Ȩ",
    "ȩ",
    "Ȫ",
    "ȫ",
    "Ȭ",
    "ȭ",
    "Ȯ",
    "ȯ",
    "Ȱ",
    "ȱ",
    "Ȳ",
    "ȳ",
    "ɐ",
    "ɑ",
    "ɒ",
    "ɓ",
    "ɔ",
    "ɕ",
    "ɖ",
    "ɗ",
    "ɘ",
    "ə",
    "ɚ",
    "ɛ",
    "ɜ",
    "ɝ",
    "ɞ",
    "ɟ",
    "ɠ",
    "ɡ",
    "ɢ",
    "ɣ",
    "ɤ",
    "ɥ",
    "ɦ",
    "ɧ",
    "ɨ",
    "ɩ",
    "ɪ",
    "ɫ",
    "ɬ",
    "ɭ",
    "ɮ",
    "ɯ",
    "ɰ",
    "ɱ",
    "ɲ",
    "ɳ",
    "ɴ",
    "ɵ",
    "ɶ",
    "ɷ",
    "ɸ",
    "ɹ",
    "ɺ",
    "ɻ",
    "ɼ",
    "ɽ",
    "ɾ",
    "ɿ",
    "ʀ",
    "ʁ",
    "ʂ",
    "ʃ",
    "ʄ",
    "ʅ",
    "ʆ",
    "ʇ",
    "ʈ",
    "ʉ",
    "ʊ",
    "ʋ",
    "ʌ",
    "ʍ",
    "ʎ",
    "ʏ",
    "ʐ",
    "ʑ",
    "ʒ",
    "ʓ",
    "ʔ",
    "ʕ",
    "ʖ",
    "ʗ",
    "ʘ",
    "ʙ",
    "ʚ",
    "ʛ",
    "ʜ",
    "ʝ",
    "ʞ",
    "ʟ",
    "ʠ",
    "ʡ",
    "ʢ",
    "ʣ",
    "ʤ",
    "ʥ",
    "ʦ",
    "ʧ",
    "ʨ",
    "ʩ",
    "ʪ",
    "ʫ",
    "ʬ",
    "ʭ",
    "ʰ",
    "ʱ",
    "ʲ",
    "ʳ",
    "ʴ",
    "ʵ",
    "ʶ",
    "ʷ",
    "ʸ",
    "Ά",
    "Έ",
    "Ή",
    "Ί",
    "Ό",
    "Ύ",
    "Ώ",
    "ΐ",
    "Α",
    "Β",
    "Γ",
    "Δ",
    "Ε",
    "Ζ",
    "Η",
    "Θ",
    "Ι",
    "Κ",
    "Λ",
    "Μ",
    "Ν",
    "Ξ",
    "Ο",
    "Π",
    "Ρ",
    "Σ",
    "Τ",
    "Υ",
    "Φ",
    "Χ",
    "Ψ",
    "Ω",
    "Ϊ",
    "Ϋ",
    "ά",
    "έ",
    "ή",
    "ί",
    "ΰ",
    "α",
    "β",
    "γ",
    "δ",
    "ε",
    "ζ",
    "η",
    "θ",
    "ι",
    "κ",
    "λ",
    "μ",
    "ν",
    "ξ",
    "ο",
    "π",
    "ρ",
    "ς",
    "σ",
    "τ",
    "υ",
    "φ",
    "χ",
    "ψ",
    "ω",
    "ϊ",
    "ϋ",
    "ό",
    "ύ",
    "ώ",
    "ϕ",
    "ϖ",
    "ϲ",
    "ϳ",
    "ϴ",
    "϶",
    "Ѐ",
    "Ё",
    "Ђ",
    "Ѓ",
    "Є",
    "Ѕ",
    "І",
    "Ї",
    "Ј",
    "Љ",
    "Њ",
    "Ћ",
    "Ќ",
    "Ѝ",
    "Ў",
    "Џ",
    "А",
    "Б",
    "В",
    "Г",
    "Д",
    "Е",
    "Ж",
    "З",
    "И",
    "Й",
    "К",
    "Л",
    "М",
    "Н",
    "О",
    "П",
    "Р",
    "С",
    "Т",
    "У",
    "Ф",
    "Х",
    "Ц",
    "Ч",
    "Ш",
    "Щ",
    "Ъ",
    "Ы",
    "Ь",
    "Э",
    "Ю",
    "Я",
    "а",
    "б",
    "в",
    "г",
    "д",
    "е",
    "ж",
    "з",
    "и",
    "й",
    "к",
    "л",
    "м",
    "н",
    "о",
    "п",
    "р",
    "с",
    "т",
    "у",
    "ф",
    "х",
    "ц",
    "ч",
    "ш",
    "щ",
    "ъ",
    "ы",
    "ь",
    "э",
    "ю",
    "я",
    "ѐ",
    "ё",
    "ђ",
    "ѓ",
    "є",
    "ѕ",
    "і",
    "ї",
    "ј",
    "љ",
    "њ",
    "ћ",
    "ќ",
    "ѝ",
    "ў",
    "џ",
    "Ѧ",
    "ѧ",
    "Ѩ",
    "ѩ",
    "Ѫ",
    "ѫ",
    "Ѭ",
    "ѭ",
    "Ѳ",
    "ѳ",
    "Ѵ",
    "ѵ",
    "Ѷ",
    "ѷ",
    "Ѹ",
    "ѹ",
    "Ґ",
    "ґ",
    "Ғ",
    "ғ",
    "Ҕ",
    "ҕ",
    "Җ",
    "җ",
    "Ҙ",
    "ҙ",
    "Қ",
    "қ",
    "Ҡ",
    "ҡ",
    "Ң",
    "ң",
    "Ҥ",
    "ҥ",
    "Ҫ",
    "ҫ",
    "Ҭ",
    "ҭ",
    "Ү",
    "ү",
    "Ұ",
    "ұ",
    "Ҳ",
    "ҳ",
    "Ҷ",
    "ҷ",
    "Ҹ",
    "ҹ",
    "һ",
    "Ӏ",
    "Ӂ",
    "ӂ",
    "Ӈ",
    "ӈ",
    "Ӌ",
    "ӌ",
    "Ӑ",
    "ӑ",
    "Ӓ",
    "ӓ",
    "Ӕ",
    "ӕ",
    "Ӗ",
    "ӗ",
    "Ә",
    "ә",
    "Ӛ",
    "ӛ",
    "Ӝ",
    "ӝ",
    "Ӟ",
    "ӟ",
    "Ӡ",
    "ӡ",
    "Ӣ",
    "ӣ",
    "Ӥ",
    "ӥ",
    "Ӧ",
    "ӧ",
    "Ө",
    "ө",
    "Ӫ",
    "ӫ",
    "Ӭ",
    "ӭ",
    "Ӯ",
    "ӯ",
    "Ӱ",
    "ӱ",
    "Ӳ",
    "ӳ",
    "Ӵ",
    "ӵ",
    "Ӹ",
    "ӹ",
    "฿",
    "Ḁ",
    "ḁ",
    "Ḃ",
    "ḃ",
    "Ḅ",
    "ḅ",
    "Ḇ",
    "ḇ",
    "Ḉ",
    "ḉ",
    "Ḋ",
    "ḋ",
    "Ḍ",
    "ḍ",
    "Ḏ",
    "ḏ",
    "Ḑ",
    "ḑ",
    "Ḓ",
    "ḓ",
    "Ḕ",
    "ḕ",
    "Ḗ",
    "ḗ",
    "Ḙ",
    "ḙ",
    "Ḛ",
    "ḛ",
    "Ḝ",
    "ḝ",
    "Ḟ",
    "ḟ",
    "Ḡ",
    "ḡ",
    "Ḣ",
    "ḣ",
    "Ḥ",
    "ḥ",
    "Ḧ",
    "ḧ",
    "Ḩ",
    "ḩ",
    "Ḫ",
    "ḫ",
    "Ḭ",
    "ḭ",
    "Ḯ",
    "ḯ",
    "Ḱ",
    "ḱ",
    "Ḳ",
    "ḳ",
    "Ḵ",
    "ḵ",
    "Ḷ",
    "ḷ",
    "Ḹ",
    "ḹ",
    "Ḻ",
    "ḻ",
    "Ḽ",
    "ḽ",
    "Ḿ",
    "ḿ",
    "Ṁ",
    "ṁ",
    "Ṃ",
    "ṃ",
    "Ṅ",
    "ṅ",
    "Ṇ",
    "ṇ",
    "Ṉ",
    "ṉ",
    "Ṋ",
    "ṋ",
    "Ṍ",
    "ṍ",
    "Ṏ",
    "ṏ",
    "Ṑ",
    "ṑ",
    "Ṓ",
    "ṓ",
    "Ṕ",
    "ṕ",
    "Ṗ",
    "ṗ",
    "Ṙ",
    "ṙ",
    "Ṛ",
    "ṛ",
    "Ṝ",
    "ṝ",
    "Ṟ",
    "ṟ",
    "Ṡ",
    "ṡ",
    "Ṣ",
    "ṣ",
    "Ṥ",
    "ṥ",
    "Ṧ",
    "ṧ",
    "Ṩ",
    "ṩ",
    "Ṫ",
    "ṫ",
    "Ṭ",
    "ṭ",
    "Ṯ",
    "ṯ",
    "Ṱ",
    "ṱ",
    "Ṳ",
    "ṳ",
    "Ṵ",
    "ṵ",
    "Ṷ",
    "ṷ",
    "Ṹ",
    "ṹ",
    "Ṻ",
    "ṻ",
    "Ṽ",
    "ṽ",
    "Ṿ",
    "ṿ",
    "Ẁ",
    "ẁ",
    "Ẃ",
    "ẃ",
    "Ẅ",
    "ẅ",
    "Ẇ",
    "ẇ",
    "Ẉ",
    "ẉ",
    "Ẋ",
    "ẋ",
    "Ẍ",
    "ẍ",
    "Ẏ",
    "ẏ",
    "Ẑ",
    "ẑ",
    "Ẓ",
    "ẓ",
    "Ẕ",
    "ẕ",
    "ẖ",
    "ẗ",
    "ẘ",
    "ẙ",
    "ẚ",
    "ẛ",
    "ẞ",
    "Ạ",
    "ạ",
    "Ả",
    "ả",
    "Ấ",
    "ấ",
    "Ầ",
    "ầ",
    "Ẩ",
    "ẩ",
    "Ẫ",
    "ẫ",
    "Ậ",
    "ậ",
    "Ắ",
    "ắ",
    "Ằ",
    "ằ",
    "Ẳ",
    "ẳ",
    "Ẵ",
    "ẵ",
    "Ặ",
    "ặ",
    "Ẹ",
    "ẹ",
    "Ẻ",
    "ẻ",
    "Ẽ",
    "ẽ",
    "Ế",
    "ế",
    "Ề",
    "ề",
    "Ể",
    "ể",
    "Ễ",
    "ễ",
    "Ệ",
    "ệ",
    "Ỉ",
    "ỉ",
    "Ị",
    "ị",
    "Ọ",
    "ọ",
    "Ỏ",
    "ỏ",
    "Ố",
    "ố",
    "Ồ",
    "ồ",
    "Ổ",
    "ổ",
    "Ỗ",
    "ỗ",
    "Ộ",
    "ộ",
    "Ớ",
    "ớ",
    "Ờ",
    "ờ",
    "Ở",
    "ở",
    "Ỡ",
    "ỡ",
    "Ợ",
    "ợ",
    "Ụ",
    "ụ",
    "Ủ",
    "ủ",
    "Ứ",
    "ứ",
    "Ừ",
    "ừ",
    "Ử",
    "ử",
    "Ữ",
    "ữ",
    "Ự",
    "ự",
    "Ỳ",
    "ỳ",
    "Ỵ",
    "ỵ",
    "Ỷ",
    "ỷ",
    "Ỹ",
    "ỹ",
    "ἀ",
    "ἁ",
    "ἂ",
    "ἃ",
    "ἄ",
    "ἅ",
    "ἆ",
    "ἇ",
    "Ἀ",
    "Ἁ",
    "Ἂ",
    "Ἃ",
    "Ἄ",
    "Ἅ",
    "Ἆ",
    "Ἇ",
    "ἐ",
    "ἑ",
    "ἒ",
    "ἓ",
    "ἔ",
    "ἕ",
    "Ἐ",
    "Ἑ",
    "Ἒ",
    "Ἓ",
    "Ἔ",
    "Ἕ",
    "ἠ",
    "ἡ",
    "ἢ",
    "ἣ",
    "ἤ",
    "ἥ",
    "ἦ",
    "ἧ",
    "Ἠ",
    "Ἡ",
    "Ἢ",
    "Ἣ",
    "Ἤ",
    "Ἥ",
    "Ἦ",
    "Ἧ",
    "ἰ",
    "ἱ",
    "ἲ",
    "ἳ",
    "ἴ",
    "ἵ",
    "ἶ",
    "ἷ",
    "Ἰ",
    "Ἱ",
    "Ἲ",
    "Ἳ",
    "Ἴ",
    "Ἵ",
    "Ἶ",
    "Ἷ",
    "ὀ",
    "ὁ",
    "ὂ",
    "ὃ",
    "ὄ",
    "ὅ",
    "Ὀ",
    "Ὁ",
    "Ὂ",
    "Ὃ",
    "Ὄ",
    "Ὅ",
    "ὐ",
    "ὑ",
    "ὒ",
    "ὓ",
    "ὔ",
    "ὕ",
    "ὖ",
    "ὗ",
    "Ὑ",
    "Ὓ",
    "Ὕ",
    "Ὗ",
    "ὠ",
    "ὡ",
    "ὢ",
    "ὣ",
    "ὤ",
    "ὥ",
    "ὦ",
    "ὧ",
    "Ὠ",
    "Ὡ",
    "Ὢ",
    "Ὣ",
    "Ὤ",
    "Ὥ",
    "Ὦ",
    "Ὧ",
    "ὰ",
    "ά",
    "ὲ",
    "έ",
    "ὴ",
    "ή",
    "ὶ",
    "ί",
    "ὸ",
    "ό",
    "ὺ",
    "ύ",
    "ὼ",
    "ώ",
    "ᾀ",
    "ᾁ",
    "ᾂ",
    "ᾃ",
    "ᾄ",
    "ᾅ",
    "ᾆ",
    "ᾇ",
    "ᾈ",
    "ᾉ",
    "ᾊ",
    "ᾋ",
    "ᾌ",
    "ᾍ",
    "ᾎ",
    "ᾏ",
    "ᾐ",
    "ᾑ",
    "ᾒ",
    "ᾓ",
    "ᾔ",
    "ᾕ",
    "ᾖ",
    "ᾗ",
    "ᾘ",
    "ᾙ",
    "ᾚ",
    "ᾛ",
    "ᾜ",
    "ᾝ",
    "ᾞ",
    "ᾟ",
    "ᾠ",
    "ᾡ",
    "ᾢ",
    "ᾣ",
    "ᾤ",
    "ᾥ",
    "ᾦ",
    "ᾧ",
    "ᾨ",
    "ᾩ",
    "ᾪ",
    "ᾫ",
    "ᾬ",
    "ᾭ",
    "ᾮ",
    "ᾯ",
    "ᾰ",
    "ᾱ",
    "ᾲ",
    "ᾳ",
    "ᾴ",
    "ᾶ",
    "ᾷ",
    "Ᾰ",
    "Ᾱ",
    "Ὰ",
    "Ά",
    "ᾼ",
    "ῂ",
    "ῃ",
    "ῄ",
    "ῆ",
    "ῇ",
    "Ὲ",
    "Έ",
    "Ὴ",
    "Ή",
    "ῌ",
    "ῐ",
    "ῑ",
    "ῒ",
    "ΐ",
    "ῖ",
    "ῗ",
    "Ῐ",
    "Ῑ",
    "Ὶ",
    "Ί",
    "ῠ",
    "ῡ",
    "ῢ",
    "ΰ",
    "ῤ",
    "ῥ",
    "ῦ",
    "ῧ",
    "Ῠ",
    "Ῡ",
    "Ὺ",
    "Ύ",
    "Ῥ",
    "ῲ"
    "ῳ",
    "ῴ",
    "ῶ",
    "ῷ",
    "Ὸ",
    "Ό",
    "Ὼ",
    "Ώ",
    "ῼ",
};

size_t getSymbolCount() {
    return _symbols.size();
}

std::string getSymbol(size_t index) {
    return _symbols.at(index);
}

}  // namespace tmbasic

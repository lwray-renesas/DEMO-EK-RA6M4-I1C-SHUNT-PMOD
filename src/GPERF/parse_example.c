/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: 'C:\\ProgramData\\chocolatey\\lib\\gperf\\tools\\gperf.exe' -t --output-file=parse_example.c tokens.gperf  */
/* Computed positions: -k'7' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "tokens.gperf"
struct parameter_list { char *param; rl78_i1c_message_key key_value; };

#define TOTAL_KEYWORDS 17
#define MIN_WORD_LENGTH 11
#define MAX_WORD_LENGTH 21
#define MIN_HASH_VALUE 11
#define MAX_HASH_VALUE 47
/* maximum key range = 37, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 30, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 25, 48, 48, 48, 48,
      20, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48,  0, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48,  0, 48, 48, 48, 48, 48, 48, 48, 48,
      20, 48, 48, 48,  8, 48, 10, 48,  0, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
      48, 48, 48, 48, 48, 48
    };
  return len + asso_values[(unsigned char)str[6]];
}

struct parameter_list *
in_word_set (register const char *str, register size_t len)
{
  static struct parameter_list wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 3 "tokens.gperf"
      {"Voltage RMS", V_RMS},
      {""}, {""},
#line 8 "tokens.gperf"
      {"Reactive Power", REACT_POWER},
      {""}, {""},
#line 7 "tokens.gperf"
      {"Fundamental Power", FUND_POWER},
      {""},
#line 17 "tokens.gperf"
      {"Reactive Max Demand", REACT_MAX_DEMAND},
      {""},
#line 14 "tokens.gperf"
      {"Total Reactive Energy", TOTAL_REACT_ENERGY},
#line 12 "tokens.gperf"
      {"Line Frequency", LINE_FREQ},
      {""},
#line 5 "tokens.gperf"
      {"Current RMS CT", I_RMS_CT},
      {""}, {""},
#line 4 "tokens.gperf"
      {"Current RMS Shunt", I_RMS_SHUNT},
      {""},
#line 19 "tokens.gperf"
      {"Ambient Temperature", AMB_TEMP},
      {""}, {""},
#line 10 "tokens.gperf"
      {"Power Factor", P_FACT},
      {""},
#line 9 "tokens.gperf"
      {"Apparent Power", APPAR_POWER},
      {""}, {""},
#line 11 "tokens.gperf"
      {"Power Factor Sign", P_FACT_SIGN},
      {""},
#line 18 "tokens.gperf"
      {"Apparent Max Demand", APPAR_MAX_DEMAND},
      {""}, {""},
#line 6 "tokens.gperf"
      {"Active Power", ACT_POWER},
      {""},
#line 13 "tokens.gperf"
      {"Total Active Energy", TOTAL_ACT_ENERGY},
      {""},
#line 15 "tokens.gperf"
      {"Total Apparent Energy", TOTAL_APPAR_ENERGY},
#line 16 "tokens.gperf"
      {"Active Max Demand", ACT_MAX_DEMAND}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}

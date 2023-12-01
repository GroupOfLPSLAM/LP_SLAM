#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <array>
#include <codecvt>
#include <locale>

int LevenshteinDistance(const std::wstring& s1, const std::wstring& s2) {
    const int m = s1.size();
    const int n = s2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    for (int i = 1; i <= m; ++i) {
        dp[i][0] = i;
    }
    for (int j = 1; j <= n; ++j) {
        dp[0][j] = j;
    }

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else {
                dp[i][j] = std::min(dp[i - 1][j - 1], std::min(dp[i][j - 1], dp[i - 1][j])) + 1;
            }
        }
    }

    return dp[m][n];
}

/*

std::string utf8_encode(const std::wstring& wstr) {
    std::string result;
    const auto len = wstr.length();
    result.reserve(len * 3);

    for (std::size_t i = 0; i < len; ++i) {
        const auto wc = wstr[i];
        if (wc < 0x80) {
            result.push_back(static_cast<char>(wc));
        }
        else {
            std::array<char, 4> bytes;
            const auto count = std::size(utf8::encode(wc, bytes.begin()));
            result.append(bytes.begin(), bytes.begin() + count);
        }
    }

    return result;
}
*/
#include <iostream>
#include <string>

float string_compare(std::string str1, std::string str2) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstr1 = converter.from_bytes(str1);
    std::wstring wstr2 = converter.from_bytes(str2);


    const int dist = LevenshteinDistance(wstr1, wstr2);
    int max_size = (wstr1.size() > wstr2.size())? wstr1.size() : wstr2.size();
    return 2 * (float)dist / (wstr1.size() + wstr2.size());

}

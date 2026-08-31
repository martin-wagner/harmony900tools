// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>

namespace lib
{

/*!
 * \brief Filters IR command names for those referring to "input" (as in signal
 *        input / Eingang / entrada), based on a static multilingual keyword list.
 *
 * A command name matches if any keyword occurs anywhere in the normalized
 * name (substring containment), not by exact token comparison.
 */
class InputKeywordMatcher
{
public:
    InputKeywordMatcher();

    QStringList filter(const QStringList &commandNames) const;

private:
    static QString normalize(const QString &text);
    bool containsInputKeyword(const QString &normalizedName) const;

    QSet<QString> keywords;
};

inline InputKeywordMatcher::InputKeywordMatcher()
{
    keywords = {
        // English
        "input", "inputs",
        // German (diacritics stripped, plus common ascii "ae" spelling)
        "eingang", "eingange", "eingaenge",
        // French
        "entree", "entrees",
        // Spanish / Portuguese
        "entrada", "entradas",
        // Italian
        "ingresso", "ingressi",
        // Dutch
        "ingang", "ingangen",
        // Polish
        "wejscie", "wejscia",
        // Czech / Slovak
        "vstup", "vstupy",
        // Swedish
        "ingang", "ingangar",
        // Norwegian / Danish
        "indgang", "indgange", "inngang", "innganger",
        // Finnish
        "tulo", "tulot",
        // Russian (transliterated)
        "vhod", "vhody",
        // Turkish
        "girdi", "girdiler",
        // Hungarian
        "bemenet", "bemenetek",
        // Romanian
        "intrare", "intrari",
    };
}

inline QString InputKeywordMatcher::normalize(const QString &text)
{
    QString lowered = text.toLower();
    QString decomposed = lowered.normalized(QString::NormalizationForm_KD);

    QString result;
    for (const QChar &ch : decomposed) {
        if (ch.category() != QChar::Mark_NonSpacing && ch.isLetterOrNumber()) {
            result.append(ch);
        }
    }

    return result;
}

inline bool InputKeywordMatcher::containsInputKeyword(const QString &normalizedName) const
{
    for (const QString &keyword : keywords) {
        if (normalizedName.contains(keyword)) {
            return true;
        }
    }

    return false;
}

inline QStringList InputKeywordMatcher::filter(const QStringList &commandNames) const
{
    QStringList result;

    for (const QString &name : commandNames) {
        QString normalized = normalize(name);

        if (containsInputKeyword(normalized)) {
            result.append(name);
        }
    }

    return result;
}

/*!
 * \brief Finds the best-matching candidate string for a given source string,
 *        based on the number of matching tokens.
 *
 * Tokens are split on non-alphanumeric characters, on letter/digit
 * boundaries, and on camelCase boundaries (e.g. "inputHdmi1" -> "input",
 * "hdmi", "1"). Two non-numeric tokens match if they are equal, or if one
 * contains the other and both are at least 3 characters long (this avoids
 * short tokens like "on" or "d" coincidentally matching inside unrelated
 * words). Numeric tokens must match exactly ("1" does not match "10").
 * If more than one candidate reaches the highest match count, the result
 * is considered ambiguous and an empty string is returned.
 */
class BestMatchFinder
{
  public:
    static QString findBestMatch(const QString &source,
        const QStringList &candidates);

  private:
    static QStringList tokenize(const QString &text);
    static bool isNumericToken(const QString &token);
    static bool tokensMatch(const QString &tokenA, const QString &tokenB);
    static int countMatches(const QStringList &candidateTokens,
        const QStringList &sourceTokens);
};

inline QString BestMatchFinder::findBestMatch(const QString &source,
    const QStringList &candidates)
{
  QStringList sourceTokens = tokenize(source);

  QString bestCandidate;
  int bestScore = 0;
  int bestScoreCount = 0;

  for (const QString &candidate : candidates) {
    QStringList candidateTokens = tokenize(candidate);
    int score = countMatches(candidateTokens, sourceTokens);

    if (score == 0) {
      continue;
    }

    if (score > bestScore) {
      bestScore = score;
      bestCandidate = candidate;
      bestScoreCount = 1;
    } else if (score == bestScore) {
      bestScoreCount++;
    }
  }

  if (bestScoreCount == 1) {
    return bestCandidate;
  }

  return QString();
}

inline QStringList BestMatchFinder::tokenize(const QString &text)
{
  QStringList tokens;
  QString currentToken;
  bool currentIsDigit = false;

  for (int i = 0; i < text.size(); ++i) {
    QChar ch = text.at(i);

    if (!ch.isLetterOrNumber()) {
      if (!currentToken.isEmpty()) {
        tokens.append(currentToken.toLower());
        currentToken.clear();
      }
      continue;
    }

    bool isDigit = ch.isDigit();
    bool startsNewToken = false;

    if (currentToken.isEmpty()) {
      startsNewToken = false;
    } else if (isDigit != currentIsDigit) {
      startsNewToken = true;
    } else if (ch.isUpper() && !currentIsDigit && text.at(i - 1).isLower()) {
      startsNewToken = true;
    }

    if (startsNewToken) {
      tokens.append(currentToken.toLower());
      currentToken.clear();
    }

    currentToken.append(ch);
    currentIsDigit = isDigit;
  }

  if (!currentToken.isEmpty()) {
    tokens.append(currentToken.toLower());
  }

  return tokens;
}

inline bool BestMatchFinder::isNumericToken(const QString &token)
{
  for (const QChar &ch : token) {
    if (!ch.isDigit()) {
      return false;
    }
  }

  return !token.isEmpty();
}

inline bool BestMatchFinder::tokensMatch(const QString &tokenA,
    const QString &tokenB)
{
  if (tokenA == tokenB) {
    return true;
  }

  if (isNumericToken(tokenA) || isNumericToken(tokenB)) {
    return false;
  }

  static const int minimumSubstringLength = 3;

  if (tokenA.length() < minimumSubstringLength
      || tokenB.length() < minimumSubstringLength) {
    return false;
  }

  return tokenA.contains(tokenB) || tokenB.contains(tokenA);
}

inline int BestMatchFinder::countMatches(const QStringList &candidateTokens,
    const QStringList &sourceTokens)
{
  int count = 0;

  for (const QString &candidateToken : candidateTokens) {
    for (const QString &sourceToken : sourceTokens) {
      if (tokensMatch(candidateToken, sourceToken)) {
        count++;
        break;
      }
    }
  }

  return count;
}

}

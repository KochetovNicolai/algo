#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

void suffixArray(const std::string &s, std::vector < int > &suffArray) {
  const char *str = s.c_str();
  int strSize = s.size() + 1;
  int alphabetSize = (1 << 8 * sizeof(char));
  // число элементов в каждом классе эквивалентности
  // изначально классы эквивалентности совпадают с буквами алфавита
  std::vector < int > groups(alphabetSize, 0);
  suffArray.assign(strSize, 0);
  for(int i = 0; i < strSize; i++)
    groups[(unsigned char) str[i]]++;
  // стабильная суффиксов сортировка подсчетом
  for(int i = 1; i < alphabetSize; i++)
    groups[i] += groups[i - 1];
  for(int i = strSize - 1; i >= 0; i--)
    suffArray[--groups[(unsigned char) str[i]]] = i;
  // класс эквивалентности элемента
  std::vector < int > classes(strSize, 0);
  for(int i = 1; i < strSize; i++) {
    classes[suffArray[i]] = classes[suffArray[i - 1]];
    if(str[suffArray[i]] != str[suffArray[i - 1]])
      classes[suffArray[i]]++;
  }
  // классы эквивалентности следуюших подстрок
  std::vector < int > newClasses;
  // отсортированные 2-е строки
  std::vector < int > newSuffArray(strSize);
  for(int step = 0; 1 << step < strSize; step++) {
    // сортируем 2-е строки
    for(int i = 0; i < strSize; i++)
      newSuffArray[i] = (suffArray[i] - (1 << step) + 2 * strSize) % strSize;
    // стабильно сортируем 1-е строки
    groups.assign(strSize, 0);
    for(int i = 0; i < strSize; i++)
      groups[classes[suffArray[i]]]++;
    for(int i = 1; i < groups.size(); i++)
      groups[i] += groups[i - 1];
    for(int i = strSize - 1; i >= 0; i--)
      suffArray[--groups[classes[newSuffArray[i]]]] = newSuffArray[i];
    // обновим классы эквивалентности
    newClasses.assign(strSize, 0);
    for(int i = 1; i < strSize; i++) {
      newClasses[suffArray[i]] = newClasses[suffArray[i - 1]];
      if((classes[suffArray[i]] != classes[suffArray[i - 1]]) || 
        (classes[(suffArray[i] + (1 << step)) % strSize] != classes[(suffArray[i - 1] + (1 << step)) % strSize]))
        newClasses[suffArray[i]]++;
    }
    classes.swap(newClasses);
  }
}

int binSearch(const std::string &text, const std::string &pattern, const std::vector < int > &suffArray, bool isLower) {
  const char *str = text.c_str();
  // lower : (left...right]
  // upper : [left...right)
  int left = (isLower ? 0 : 1);
  int right = text.size() + (isLower ? 0 : 1);
  int matches = 0;
  while(right - left > 1) {
    int med = (left + right) / 2;
    int medMatches;
    // сравниваем, начиная с обшего префикса индексов
    for(medMatches = matches; medMatches < pattern.size() &&
      pattern[medMatches] == str[(suffArray[med] + medMatches) % suffArray.size()]; medMatches++);
    if(isLower) {
      if(medMatches != pattern.size() && pattern[medMatches] > str[(suffArray[med] + medMatches) % suffArray.size()])
        left = med;
      else
        right = med;
    } else {
      if(medMatches != pattern.size() && pattern[medMatches] < str[(suffArray[med] + medMatches) % suffArray.size()])
        right = med;
      else
        left = med;
    }
    //пересчитываем общий префикс индексов
    while(matches < medMatches && right < suffArray.size() && 
      str[(suffArray[left] + matches) % suffArray.size()] == str[(suffArray[right] + matches) % suffArray.size()])
      matches++;
  }
  return isLower ? right : left;
}

class Functor {
    const std::string &pattern;
    const std::string &text;
public:
    bool operator()(int left, int right) {
        if (left == -1) {
            return pattern < text.substr(right, std::min(pattern.size(), text.size() - right));
        } else if (right == -1) {
            return text.substr(left, std::min(pattern.size(), text.size() - left)) < pattern;
        } else {
            return text.substr(left, std::min(pattern.size(), text.size() - left)) < 
                text.substr(right, std::min(pattern.size(), text.size() - right));
        }
    }
    Functor(const std::string &text, const std::string &pattern):
        text(text),
        pattern(pattern) {
    }
};

void findUsingSTL(const std::string &text, const std::string &pattern, std::vector < int > &positions) {
  std::vector < int > suffArray;
  suffixArray(text, suffArray);
  std::vector< int >::iterator begin;
  std::vector< int >::iterator end;
  Functor functor(text, pattern);
  begin = std::lower_bound(suffArray.begin(), suffArray.end(), -1, functor);
  end = std::upper_bound(suffArray.begin(), suffArray.end(), -1, functor);
  if (begin != suffArray.end() && begin == end && text.substr(*begin, pattern.size()) != pattern) {
      return;
  }
  positions.assign(begin, end);
}

void find(const std::string &text, const std::string &pattern, std::vector < int > &positions) {
  std::vector < int > suffArray;
  suffixArray(text, suffArray);
  int left = binSearch(text, pattern, suffArray, true);
  int right = binSearch(text, pattern, suffArray, false);
  if(left == right && text.substr(suffArray[left], pattern.size()) != pattern)
    return;
  for(int i = left; i <= right; i++)
    positions.push_back(suffArray[i]);
}

int main() {
  std::string text, pattern;
  std::cin >> text >> pattern;
  std::vector < int > positions;
  findUsingSTL(text, pattern, positions);
  std::sort(positions.begin(), positions.end());
  for(int i = 0; i < positions.size(); i++)
    std::cout << positions[i] << ' ';
  std::cout << std::endl;
  return 0;
}
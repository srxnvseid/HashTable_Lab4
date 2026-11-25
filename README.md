# VSA1 — Laboratoriya işi № 4: Heş cədvəlləri, sıxışdırma funksiyaları və kolliziyaların həlli

**Azərbaycan Respublikası Elm və Təhsil Nazirliyi**  
**Qarabağ Universiteti — Mühəndislik fakültəsi**

| Məlumat | Dəyər |
|---|---|
| **Fənn** | Verilənlər Strukturları və Alqoritmlər 1 (VSA1) |
| **Laboratoriya işi** | № 4 |
| **Qrup** | KOM25 |
| **Müəllim** | Şamil Hümbətov |
| **Tarix** | 25 noyabr 2026 |
| **Komanda** | Səid Saruxanov, Nicat `[TO BE FILLED]`, Elgün `[TO BE FILLED]` |

---

## 1. Layihənin təsviri

Bu repozitoriya C++ dilində sıfırdan yazılmış heş cədvəli (hash table) tətbiqlərini və onların empirik müqayisəsini əhatə edir. Layihədə aşağıdakı mövzular reallaşdırılıb:

- **Kolliziyaların həlli üsulları:** Separate Chaining (ayrı zəncirləmə) və Open Addressing (açıq ünvanlama — Linear Probing və Quadratic Probing).
- **Heş funksiyası:** Horner qaydası ilə polinom heş (`a = 37`, həmçinin müqayisə üçün `a = 1`).
- **Sıxışdırma funksiyaları (compression):** bölünmə üsulu (`k % N`) və MAD (*Multiply-Add-Divide*: `((a * (k % p) + b) % p) % N`).
- **Dinamik ölçü idarəetməsi:** yük faktoruna (load factor, $\lambda$) əsasən cədvəlin böyüdülməsi, kiçildilməsi və sadə ölçüyə (`nextPrime`) yenidən heşlənməsi (`rehash`).
- **Tənbəl silmə (lazy deletion):** açıq ünvanlamada axtarış zəncirini pozmadan silmə (`DELETED` işarəsi) və `rehash` zamanı təmizlənmə.
- **Empirik analiz:** 5000 unikal ISBN açarı üzərində kolliziya sayı, maksimum zəncir/klaster uzunluğu və orta axtarış addımının ölçülməsi.

Heş cədvəli məntiqi üçün `std::unordered_map`, `std::unordered_set`, `std::map` və ya oxşar hazır kitabxana konteynerlərindən istifadə olunmayıb; bütün heş strukturları sıfırdan yazılıb (`task3.cpp`-də `std::set` yalnız test üçün unikal açarların yaradılmasında istifadə olunur).

---

## 2. Repozitoriya strukturu

```text
HashTable_Lab4/
├── .gitignore    # Kompilyasiya fayllarını (*.exe, *.out, *.o) Git-dən çıxarır
├── README.md     # Layihənin sənədləşdirilməsi
├── task1.cpp     # Tapşırıq 1: Separate Chaining əsaslı kitabxana heş cədvəli
├── task2.cpp     # Tapşırıq 2: Open Addressing (Linear/Quadratic) əsaslı IP qara siyahısı
└── task3.cpp     # Tapşırıq 3: Polinom sabiti və kolliziya üsullarının empirik analizi
```

---

## 3. Reallaşdırılmış komponentlər

### Tapşırıq 1 (`task1.cpp`) — Separate Chaining

Kitabxana sistemi üçün ISBN açarına görə işləyən ayrı zəncirləmə heş cədvəli:

- **Strukturlar:**
  - `Book` — `isbn`, `title`, `author`, `isAvailable` sahələri.
  - `HashTable` — `vector<vector<Book>> table`, `currentSize`, `useMAD`, `p`, `a`, `b` sahələri. *(Qeyd: bu faylda struktur `HashTable` adlanır; `SeparateChainingHash` adı `task3.cpp`-də istifadə olunur.)*
- **Funksiyalar:**
  - `isPrime`, `nextPrime` — sadə ədəd yoxlaması və növbəti sadə ədədin tapılması.
  - `setupTable` — cədvəli ilkinləşdirir, `p = nextPrime(size + 1)`, `a ∈ [1, p - 1]` və `b ∈ [0, p - 1]` MAD parametrlərini seçir.
  - `polynomialHash` — Horner qaydası ilə polinom heş (`a = 37`).
  - `getIndex` — `useMAD == false` olduqda `k % N`, `useMAD == true` olduqda `((a * (k % p) + b) % p) % N` hesablayır.
  - `getLoadFactor` — yük faktorunu (`currentSize / table.size()`) qaytarır.
  - `insertBook` — təkrarlanan ISBN-i rədd edir, əlavə edir və `getLoadFactor(ht) >= 1.0` olduqda `nextPrime(2 * table.size())` ölçüsünə `rehash` edir.
  - `findByISBN` — ISBN üzrə axtarış aparır.
  - `removeBook` — elementi silir; `table.size() > 11` və `getLoadFactor(ht) < 0.25` olduqda `nextPrime(table.size() / 2)` (minimum `11`) ölçüsünə kiçildir.
  - `rehash` — elementləri yeni ölçülü cədvələ yenidən yerləşdirir.
- **`main()`:** `srand(time(0))` ilə MAD rejimində (`useMAD = true`, başlanğıc ölçü `nextPrime(5)`) 5 kitab əlavə edir və birini (`"333"`) silir.
- **Qeyd:** `getIndex` həm `k % N`, həm də MAD rejimini dəstəkləyir, lakin `task1.cpp` daxilində bu iki rejimin ayrıca müqayisə çıxışı yazılmayıb (`[NOT IMPLEMENTED / TO BE FILLED]`).

### Tapşırıq 2 (`task2.cpp`) — Open Addressing

IP ünvanlarının qara siyahısı (firewall) üçün açıq ünvanlama heş cədvəli:

- **Tiplər və strukturlar:**
  - `enum Status { EMPTY, ACTIVE, DELETED }`
  - `enum ProbeMode { LINEAR, QUADRATIC }`
  - `Cell` — `ip`, `status` sahələri.
  - `HashTable` — `vector<Cell> table`, `currentSize`, `mode` sahələri. *(Qeyd: bu faylda struktur `HashTable` adlanır; `OpenAddressingHash` adı `task3.cpp`-də istifadə olunur.)*
- **Funksiyalar:**
  - `isPrime`, `nextPrime`, `setupTable`, `polynomialHash` (`a = 37`), `getIndex` (`k % N`), `getLoadFactor`.
  - `getStep` — `LINEAR` rejimində `1`, `QUADRATIC` rejimində `2 * i - 1` addımı qaytarır ($H_i = (H_{i-1} + 2i - 1) \bmod N$).
  - `insertIP` — dublikat IP-ni rədd edir, rast gəlinən ilk `DELETED` xananı (`firstDeleted`) təkrar istifadə edir, `getLoadFactor(ht) > 0.5` olduqda (və ya boş/silinmiş yer tapılmadıqda) `nextPrime(2 * table.size())` ölçüsünə `rehash` edir.
  - `containsIP` — axtarış zamanı `DELETED` xanalarından keçir, `EMPTY` gördükdə dayanır və yoxlama sayını `probesOut` vasitəsilə qaytarır.
  - `removeIP` — tənbəl silmə (lazy deletion) tətbiq edir (`status = DELETED`, `ip = ""`, `currentSize--`).
  - `rehash` — yalnız `ACTIVE` xanaları yeni cədvələ köçürərək `DELETED` qeydlərini təmizləyir.
- **`main()`:** `QUADRATIC` rejimdə `nextPrime(11)` ölçüsü ilə 6 IP əlavə edib rehash prosesini, dublikat yoxlamasını, `containsIP` probe sayını və `removeIP` əməliyyatını nümayiş etdirir.

### Tapşırıq 3 (`task3.cpp`) — Empirik analiz

Polinom sabitinin və kolliziya həlli üsullarının müqayisəsi üçün test skripti:

- **Strukturlar:** `SeparateChainingHash` (`table`, `currentSize`, `a`, `collisions`), `OpenAddressingHash` (`table`, `currentSize`, `mode`, `a`), `Cell`, `Status`, `ProbeMode`.
- **Test parametrləri:** `keyCount = 5000` unikal `"978-XXXXXXXXXX"` formatlı ISBN açarı (`generateKeys`, `seed = 42`), `tableSize = 10007`.
- **Ölçülən göstəricilər:**
  - `a = 1` (ASCII cəmi) və `a = 37` üçün Separate Chaining ümumi kolliziya sayı (`collisions`).
  - `a = 37` və `TableSize = 10007` üçün Separate Chaining (`getMaxChain`, `getAverageProbesChaining`), Linear Probing (`getMaxCluster`, `getAverageProbesOpen`) və Quadratic Probing (`getMaxCluster`, `getAverageProbesOpen`).
- **Ölçmə nəticələri və qrafiklər:** Repozitoriyada saxlanmayıb — `[TO BE FILLED]`.

---

## 4. Kompilyasiya və işə salma

Hər tapşırıq ayrıca C++17 proqramı kimi kompilyasiya olunur.

**Windows (PowerShell):**

```powershell
g++ -std=c++17 task1.cpp -o task1.exe
.\task1.exe

g++ -std=c++17 task2.cpp -o task2.exe
.\task2.exe

g++ -std=c++17 task3.cpp -o task3.exe
.\task3.exe
```

**Linux / macOS:**

```bash
g++ -std=c++17 task1.cpp -o task1
./task1

g++ -std=c++17 task2.cpp -o task2
./task2

g++ -std=c++17 task3.cpp -o task3
./task3
```

---

## 5. İş bölgüsü

> **Qeyd:** Bu bölmə komanda tərəfindən təsdiqlənməlidir (`[TO BE CONFIRMED]`).

- **Səid Saruxanov:** Tapşırıq 1 (`Separate Chaining`, polinom heş, `k mod N` və MAD sıxışdırma funksiyalarının müqayisəsi, `resize`/`shrink`); hesabatda arxitektura və `MAD` vs `k mod N` bölməsi.
- **Nicat `[TO BE FILLED]`:** Tapşırıq 2 (`Open Addressing`, `ACTIVE`/`EMPTY`/`DELETED` vəziyyətləri, `LINEAR`/`QUADRATIC` probing, tənbəl silmə, $\lambda > 0.5$ zamanı `rehash`); hesabatda `DELETED` işarəsinin axtarış zəncirini necə qoruduğunu göstərən diaqram.
- **Elgün `[TO BE FILLED]`:** Tapşırıq 3 (test skripti, ölçmələr, qrafiklər); hesabatda eksperimental nəticələr və mühəndislik rəyi bölmələri.
- **Ortaq işlər:** Titul vərəqi, mənbələr, bəyannamələr. Kəsişən qaralamalar (MAD, `rehash`, yük faktoru, probing, tənbəl silmə) qarşılıqlı yoxlanılıb (cross-review): yekun versiyaya bir üzv cavabdeh olub, digəri kodu yoxlayıb və test edib.

| Tələbə | Görülən iş | Pay (%) |
|---|---|---|
| Səid Saruxanov | Tapşırıq 1 (`task1.cpp`), arxitektura və MAD vs `k mod N` hesabat bölməsi, ortaq bölmələr və cross-review | `[TO BE FILLED]` |
| Nicat `[TO BE FILLED]` | Tapşırıq 2 (`task2.cpp`), `DELETED` axtarış zənciri diaqramı, ortaq bölmələr və cross-review | `[TO BE FILLED]` |
| Elgün `[TO BE FILLED]` | Tapşırıq 3 (`task3.cpp`), ölçmələr, qrafiklər, eksperimental nəticələr və mühəndislik rəyi, ortaq bölmələr və cross-review | `[TO BE FILLED]` |

---

## 6. Mənbələr

- **GeeksforGeeks:** C++ dilində heş cədvəlləri üzrə izahlar (hash tables, separate chaining, open addressing, linear/quadratic probing, rehashing).
- **VisuAlgo:** Hash Table interaktiv vizuallaşdırması — https://visualgo.net/en/hashtable

---

## 7. Sİ İstifadə Bəyannaməsi

Bütün komanda üzvləri eyni Sİ alətlərindən eyni məqsədlərlə istifadə edib:

- **Antigravity IDE (daxili Sİ):** kodun GitHub-a yüklənməsi (`push`), kompilyasiya və sazlama (debugging); prompt istiqaməti — kompilyator xətalarının və nasazlıqların səbəbini tapmaq.
- **Gemini Hydrogen:** funksiyaların izahı, mövzunun öyrənilməsi, xətaların aradan qaldırılması, nəzəri biliklərin möhkəmləndirilməsi.
- **Claude Sonnet 5.5:** eyni məqsədlər (`removeBook`, `rehash` və MAD funksiyalarının izahı, kodun düzgünlüyünün yoxlanması, nəzəri izahlar, iş bölgüsünün müzakirəsi).

Əsas alqoritmik məntiq (`insert`, `remove`, axtarış/probing, `rehash`) **Sİ tərəfindən yazılmayıb**. Kod komanda üzvləri tərəfindən yazılıb; Sİ alətləri yalnız öyrənmə, izah və xəta araşdırması üçün istifadə olunub. (Yalnız bir kiçik əlavə — `task1.cpp`-də `#include <ctime>` ilə `srand(time(0))` — Claude tərəfindən təklif edilib.)

Hər bir komanda üzvü bütün kod bazasını sərbəst şəkildə izah edə bilir.

---

## 8. README müəllifliyi haqqında qeyd

Bu `README.md` faylı repozitoriyanın məzmunu və komandanın təqdim etdiyi məlumatlar əsasında Antigravity IDE-nin Sİ köməkçisi tərəfindən yaradılıb. Komanda tərəfindən nəzərdən keçirilib.

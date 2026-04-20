# Turnaj v třídění velkých dat

Zveme vás na neformální programátorský turnaj zaměřený na návrh
a optimalizaci třídicích algoritmů nad velkými daty.

Cílem je navrhnout co nejrychlejší řešení v daném modelu výpočtu.
Nejde jen o znalost algoritmů,
ale o schopnost pochopit nákladový model, pracovat s pamětí a efektivně paralelizovat.

## Zadání

Máte k dispozici dataset náhodných dat o velikosti řádově desítek GB.

Vaším úkolem je tato data seřadit vzestupně pomocí připraveného API.

## Dostupné operace (API)

Všichni soutěžící mají k dispozici stejné rozhraní:

* `compare(a, b)`
  Porovná dva prvky a vrátí jejich uspořádání.

* `swap(i, j)`
  Prohodí dva prvky v hlavním poli.

* `copy(src_index, dst_index, direction)`
  Zkopíruje prvek mezi hlavním a pomocným polem.

## Výpočetní model

* Operace mají stabilní a konzistentní čas.
* Přístup k datům je možný pouze přes API.
* Implementace API je pro všechny stejná.
* Dataset je pro všechny běhy identický.

## Soutěžní kategorie

### 1) In-place Comparator (Single Thread)

**Omezení:**

* pouze operace `compare` a `swap`
* žádná pomocná paměť
* pouze jedno vlákno

**Cíl:**

* navrhnout co nejefektivnější in-place třídění

### 2) Comparator (Parallel + Auxiliary Memory)

**Omezení:**

* lze použít `compare`, `swap`, `copy`
* k dispozici pomocné pole velikosti N
* paralelizace povolena

**Cíl:**

* maximalizovat výkon pomocí paralelizace a práce s pamětí

### 3) Free (No Comparator Restriction)

**Omezení:**

* lze využít všechny operace
* není nutné používat `compare`, `swap`, `copy`
* k dispozici pomocné pole velikosti N
* paralelizace povolena

**Cíl:**

* využít vlastnosti dat (integer) pro co nejrychlejší třídění

## Hodnocení

* Měří se reálný čas běhu (wall time) na referenčním stroji.
* Každé řešení bude spuštěno několikrát, počítá se nejlepší nebo průměrný čas (upřesní se).
* Dataset je fixní a stejný pro všechny.

## Prostředí

* Všichni soutěžící poběží na stejném referenčním stroji
* Stejný počet jader
* Stejné runtime prostředí
* Stejná implementace API

## Poznámky

* Kategorie jsou nezávislé — každá testuje jiný přístup.

## Cíl

Pobavit se, porovnat přístupy a zjistit, kdo dokáže nejlépe využít daný model.

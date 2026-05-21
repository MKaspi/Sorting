# Sorting

Projekt obsahuje benchmarkovaci framework pro porovnavani tridicich algoritmu. Jadro programu nacita tridici algoritmy jako pluginy ze sdilenych knihoven, spousti je nad pripravenym datasetem a umi zapisovat log operaci pro naslednou vizualizaci.

## Struktura projektu

```text
core/       jadro benchmarkeru a CLI
include/    verejna hlavickova rozhrani pro pluginy
plugins/    ukazkove tridici algoritmy
scripts/    vizualizery a pomocne skripty
Makefile    hlavni vstup pro build a benchmarky
```

Adresare `input/`, `.build/` a binarka `benchmarker` nejsou soucasti gitu.
Jsou uvedene v `.gitignore`, proto je potreba si vstupni datasety a program pripravit lokalne.

# Dokumentace

## Prace s datasety

Framework podporuje dva typy datasetu:

* dataset ulozeny v souboru
* interni dataset alokovany pouze v pameti

Souborovy dataset se pouziva hlavne pro benchmarky, opakovatelne testy a vizualizaci. Vychozi konfigurace Makefile ocekava soubor:

```text
input/dataset.int
```

### Generovani datasetu do souboru

Nejdrive sestavte projekt:

```bash
make clean
make build
```

Potom vytvorte adresar pro vstupy a vygenerujte vyvojovy dataset:

```bash
mkdir -p input
./benchmarker --dataset input/dataset.int --elem-count 1000 --enlarge-dataset --dataset-fill
```

Volba `--dataset-fill` naplni dataset nahodnymi hodnotami. Volba `--enlarge-dataset` zajisti, ze se soubor zvetsi na pozadovanou velikost, pokud jeste neexistuje nebo je prilis maly.

### Interni dataset v pameti

Pri vyvoji nebo rychlem debugovani lze vyuzit i interni dataset alokovany pouze pomoci `malloc`, bez vytvareni souboru na disku.

To je vhodne hlavne pro:

* rychle lokalni testy
* debug jednoho algoritmu
* experimenty bez pripravy vstupnich souboru

V takovem pripade benchmarker pracuje pouze s daty v pameti a neni potreba pripravovat adresar `input/`.

## Pridani vlastni implementace sort algoritmu

Novy algoritmus pridejte jako novy .c soubor do adresare `plugins/`.

Pri dalsim spusteni:

```bash
make build
```

se plugin automaticky prelozi jako sdilena knihovna do:

```text
.build/plugins/
```

Kazdy plugin implementuje rozhrani definovane v hlavickach v adresari:

```text
include/
```

Po buildu lze plugin spustit primo pres benchmarker:

```bash
./benchmarker .build/plugins/quick.so --dataset input/d2.int --aux-dataset .build/aux.file --elem-count 200 --enlarge-dataset --print-steps
```

Volba `--print-steps` vypisuje operace `compare`, `swap` a `copy`. Tyto logy se nasledne pouzivaji pro HTML i video vizualizaci.

## Vizualizery

Projekt obsahuje dva ruzne vizualizery:

* video vizualizer
* HTML debug vizualizer

Kazdy slouzi k trochu jinemu ucelu.

### HTML debug vizualizer

Skript:

```text
scripts/visualize_sorting.py
```

Toto je hlavni nastroj pro vyvoj a debug tridicich algoritmu.

Vizualizer umoznuje:

* krokovat jednotlive operace
* sledovat zmeny pole v case
* zobrazovat praci s pomocnym polem
* analyzovat chyby v algoritmu
* ladit nestandardni chovani

Typicke pouziti:

```bash
python3 scripts/visualize_sorting.py .build/bench/quick_200/output/sort.log -o quick_200.html
```

Vysledny HTML soubor otevrete v prohlizeci.

### Video vizualizer

Skript:

```text
scripts/render_sort_video.py
```

Slouzi k vytvoreni MP4 videa z logu operaci.

Makefile ho vola automaticky pri cilich:

```bash
make sort-videos
```

Rucni pouziti:

```bash
python3 scripts/render_sort_video.py .build/bench/quick_200/output/sort.log -o quick_200.mp4 --operations-to-frame 1000
```

Pro video vystup je potreba:

* Python modul `pillow`
* nastroj `ffmpeg`

Video vizualizer je vhodny hlavne pro:

* prezentaci algoritmu
* porovnani vice trideni
* export vystupu
* demonstraci prubehu benchmarku

## Makefile

Makefile je hlavni vstup pro build, benchmarky a generovani vystupu.

Vsechny prikazy spoustejte z korenoveho adresare projektu.

### Zakladni cile

```bash
make build
```

Sestavi binarku `benchmarker` a vsechny pluginy.

```bash
make clean
```

Smaze build vystupy a binarku `benchmarker`.

```bash
make sort-clean
```

Smaze pouze benchmark vystupy v `.build/bench/`.

```bash
make sort-list
```

Vypise seznam pluginu, datasetu a benchmarku, ktere budou spusteny.

### Benchmarkovaci cile

```bash
make sort-bench-no-video
```

Spusti benchmarky a vytvori pouze CSV vystupy bez videi.

Toto je nejrychlejsi varianta vhodna pro:

* cvicne testy
* rychly vyvoj
* iterativni ladeni

```bash
make sort-bench
```

Spusti benchmarky a zaroven generuje videa.

```bash
make sort-runs
```

Spusti pouze samotne benchmarky.

```bash
make sort-videos
```

Vygeneruje videa z existujicich logu.

```bash
make sort-overall
```

Vytvori souhrnna porovnavaci videa.

```bash
make sort-summary
```

Vytvori souhrnny CSV report.

### Konfigurace benchmarku

Vychozi dataset:

```text
input/dataset.int
```

Lze zmenit pomoci:

```bash
make sort-bench-no-video BENCH_INPUT=input/dataset-large.int
```

Velikosti vstupu lze upravit:

```bash
make sort-bench-no-video BENCH_ELEMS="50 200"
```


### Rychle benchmarky bez videi

```bash
make sort-bench-no-video BENCH_ELEMS="50 200"
```

### Debug konkretniho behu

```bash
python3 scripts/visualize_sorting.py .build/bench/quick_200/output/sort.log -o quick_200.html
```

### Generovani videa

```bash
make sort-bench BENCH_ELEMS="50"
```

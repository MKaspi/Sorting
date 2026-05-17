#!/usr/bin/env bash
set -euo pipefail

# Orchestrates sort runs, operation summaries, timing summaries and video rendering.
# No Python is used for orchestration. Python may be used only by the renderer.

usage() {
  cat <<'EOF'
Usage:
  run_sort_videos.sh [options]

Options:
  --bin PATH                 Bench binary. Default: ./bench
  --plugin-dir DIR           Directory with *.so plugins. Default: .build/plugins
  --input PATH               Base input dataset. Default: input/d2.int
  --out-dir DIR              Output directory. Default: .build/videos
  --elems "N1 N2 ..."        Element counts. Default: 50 200 1000
  --renderer PATH            Video renderer script. Default: ./render_sort_video_v4.py
  --operations-to-frame N    Operations per output frame. Default: 1000
  --width N                  Video width. Default: 1920
  --height N                 Video height. Default: 1080
  --fps N                    Video fps. Default: 30
  --progress-seconds N       Renderer progress period. Default: 2
  --no-video                 Run benchmarks and summaries only.
  --keep-tmp                 Keep renderer temporary files.
  --clean                    Delete output directory before running.
  --help                     Show this help.

Environment overrides are also supported:
  SORT_BIN, PLUGIN_DIR, INPUT_DATASET, OUT_DIR, VIDEO_ELEMS,
  RENDERER, OPERATIONS_TO_FRAME, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_FPS,
  PROGRESS_SECONDS, NO_VIDEO, KEEP_TMP, CLEAN_OUT
EOF
}

SORT_BIN="${SORT_BIN:-./bench}"
PLUGIN_DIR="${PLUGIN_DIR:-.build/plugins}"
INPUT_DATASET="${INPUT_DATASET:-input/d2.int}"
OUT_DIR="${OUT_DIR:-.build/videos}"
VIDEO_ELEMS="${VIDEO_ELEMS:-50 200 1000}"
RENDERER="${RENDERER:-./render_sort_video_v4.py}"
OPERATIONS_TO_FRAME="${OPERATIONS_TO_FRAME:-1000}"
VIDEO_WIDTH="${VIDEO_WIDTH:-1920}"
VIDEO_HEIGHT="${VIDEO_HEIGHT:-1080}"
VIDEO_FPS="${VIDEO_FPS:-30}"
PROGRESS_SECONDS="${PROGRESS_SECONDS:-2}"
NO_VIDEO="${NO_VIDEO:-0}"
KEEP_TMP="${KEEP_TMP:-0}"
CLEAN_OUT="${CLEAN_OUT:-0}"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --bin) SORT_BIN="$2"; shift 2 ;;
    --plugin-dir) PLUGIN_DIR="$2"; shift 2 ;;
    --input) INPUT_DATASET="$2"; shift 2 ;;
    --out-dir) OUT_DIR="$2"; shift 2 ;;
    --elems) VIDEO_ELEMS="$2"; shift 2 ;;
    --renderer) RENDERER="$2"; shift 2 ;;
    --operations-to-frame) OPERATIONS_TO_FRAME="$2"; shift 2 ;;
    --width) VIDEO_WIDTH="$2"; shift 2 ;;
    --height) VIDEO_HEIGHT="$2"; shift 2 ;;
    --fps) VIDEO_FPS="$2"; shift 2 ;;
    --progress-seconds) PROGRESS_SECONDS="$2"; shift 2 ;;
    --no-video) NO_VIDEO=1; shift ;;
    --keep-tmp) KEEP_TMP=1; shift ;;
    --clean) CLEAN_OUT=1; shift ;;
    --help) usage; exit 0 ;;
    *) echo "Unknown argument: $1" >&2; usage >&2; exit 2 ;;
  esac
done

if [ ! -x "$SORT_BIN" ]; then
  echo "Bench binary not found or not executable: $SORT_BIN" >&2
  exit 1
fi

if [ ! -d "$PLUGIN_DIR" ]; then
  echo "Plugin directory not found: $PLUGIN_DIR" >&2
  exit 1
fi

if [ ! -f "$INPUT_DATASET" ]; then
  echo "Input dataset not found: $INPUT_DATASET" >&2
  exit 1
fi

if [ "$NO_VIDEO" != "1" ] && [ ! -f "$RENDERER" ]; then
  echo "Renderer not found: $RENDERER" >&2
  exit 1
fi

if [ "$CLEAN_OUT" = "1" ]; then
  rm -rf "$OUT_DIR"
fi

mkdir -p "$OUT_DIR"
SUMMARY_CSV="$OUT_DIR/summary.csv"
RUN_LOG="$OUT_DIR/run.log"

printf 'sort,elements,operations,compare,swap,copy,unknown,wall_seconds,user_seconds,system_seconds,cpu_percent,max_rss_kb,log_bytes,video_path,work_dir\n' > "$SUMMARY_CSV"
: > "$RUN_LOG"

log_msg() {
  printf '[%(%Y-%m-%d %H:%M:%S)T] %s\n' -1 "$*" | tee -a "$RUN_LOG"
}

csv_escape() {
  # Print one CSV field.
  local s
  s=${1//\"/\"\"}
  printf '"%s"' "$s"
}

read_time_field() {
  local file="$1"
  local key="$2"
  awk -F= -v k="$key" '$1 == k {print $2; found=1} END {if (!found) print ""}' "$file"
}

count_operations() {
  local log_file="$1"
  awk '
    NR == 1 { next }
    $1 == "compare" { compare++; operations++; next }
    $1 == "swap" { swap++; operations++; next }
    $1 == "copy" { copy++; operations++; next }
    NF > 0 { unknown++ }
    END {
      printf "%d %d %d %d %d", operations, compare, swap, copy, unknown
    }
  ' "$log_file"
}

plugin_count=0
shopt -s nullglob
plugins=("$PLUGIN_DIR"/*.so)
shopt -u nullglob

if [ "${#plugins[@]}" -eq 0 ]; then
  echo "No plugins found in $PLUGIN_DIR" >&2
  exit 1
fi

for plugin in "${plugins[@]}"; do
  sort_name=$(basename "$plugin" .so)
  plugin_count=$((plugin_count + 1))

  for elem_count in $VIDEO_ELEMS; do
    run_name="${sort_name}_${elem_count}"
    work_dir="$OUT_DIR/$run_name"
    frames_tmp="$work_dir/render_tmp"
    dataset="$work_dir/input.int"
    aux_dataset="$work_dir/aux.file"
    log_file="$work_dir/sort.log"
    time_file="$work_dir/time.txt"
    video_file="$work_dir/${run_name}.mp4"

    mkdir -p "$work_dir"
    cp "$INPUT_DATASET" "$dataset"

    log_msg "run start: sort=$sort_name elements=$elem_count"

    /usr/bin/time -o "$time_file" -f 'wall_seconds=%e\nuser_seconds=%U\nsystem_seconds=%S\ncpu_percent=%P\nmax_rss_kb=%M\nio_inputs=%I\nio_outputs=%O\ncontext_switches=%c' \
      "$SORT_BIN" "$plugin" \
        --dataset "$dataset" \
        --aux-dataset "$aux_dataset" \
        --elem-count "$elem_count" \
        --enlarge-dataset \
        --print-steps \
        > "$log_file" \
        2> "$work_dir/stderr.txt"

    read -r operations compare swap copy unknown < <(count_operations "$log_file")
    wall_seconds=$(read_time_field "$time_file" wall_seconds)
    user_seconds=$(read_time_field "$time_file" user_seconds)
    system_seconds=$(read_time_field "$time_file" system_seconds)
    cpu_percent=$(read_time_field "$time_file" cpu_percent)
    max_rss_kb=$(read_time_field "$time_file" max_rss_kb)
    log_bytes=$(wc -c < "$log_file" | tr -d ' ')

    log_msg "run parsed: sort=$sort_name elements=$elem_count operations=$operations compare=$compare swap=$swap copy=$copy log_bytes=$log_bytes wall=${wall_seconds}s"

    if [ "$NO_VIDEO" = "1" ]; then
      video_path=""
    else
      mkdir -p "$frames_tmp"
      log_msg "video start: sort=$sort_name elements=$elem_count out=$video_file"
      python3 "$RENDERER" "$log_file" \
        -o "$video_file" \
        --operations-to-frame "$OPERATIONS_TO_FRAME" \
        --tmp-dir "$frames_tmp" \
        --clean-tmp \
        --width "$VIDEO_WIDTH" \
        --height "$VIDEO_HEIGHT" \
        --fps "$VIDEO_FPS" \
        --title "$sort_name n=$elem_count" \
        --progress-every-seconds "$PROGRESS_SECONDS" \
        --quiet-ffmpeg
      video_path="$video_file"
      if [ "$KEEP_TMP" != "1" ]; then
        rm -rf "$frames_tmp"
      fi
      log_msg "video done: sort=$sort_name elements=$elem_count out=$video_file"
    fi

    {
      csv_escape "$sort_name"; printf ','
      printf '%s,%s,%s,%s,%s,%s,' "$elem_count" "$operations" "$compare" "$swap" "$copy" "$unknown"
      printf '%s,%s,%s,' "$wall_seconds" "$user_seconds" "$system_seconds"
      csv_escape "$cpu_percent"; printf ','
      printf '%s,%s,' "$max_rss_kb" "$log_bytes"
      csv_escape "$video_path"; printf ','
      csv_escape "$work_dir"; printf '\n'
    } >> "$SUMMARY_CSV"

    log_msg "run done: sort=$sort_name elements=$elem_count"
  done
done

log_msg "all done: plugins=$plugin_count summary=$SUMMARY_CSV"

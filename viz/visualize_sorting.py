#!/usr/bin/env python3
import argparse
import json
from pathlib import Path

HTML_TEMPLATE = r'''<!doctype html>
<html lang="cs">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Sorting visualizer</title>
<style>
:root { font-family: system-ui, -apple-system, Segoe UI, sans-serif; color-scheme: light dark; }
body { margin: 0; padding: 24px; background: Canvas; color: CanvasText; }
main { max-width: 1100px; margin: 0 auto; }
h1 { margin: 0 0 8px; font-size: 28px; }
.note { opacity: .78; margin-bottom: 18px; }
.panel { border: 1px solid color-mix(in srgb, CanvasText 20%, transparent); border-radius: 14px; padding: 16px; margin: 14px 0; }
.controls { display: grid; grid-template-columns: repeat(auto-fit, minmax(170px, 1fr)); gap: 12px; align-items: end; }
label { display: grid; gap: 6px; font-size: 14px; }
select, button, input { font: inherit; }
button, select { padding: 8px 10px; border-radius: 10px; border: 1px solid color-mix(in srgb, CanvasText 25%, transparent); background: Canvas; color: CanvasText; }
button { cursor: pointer; }
input[type="range"] { width: 100%; }
.meta { display: flex; gap: 18px; flex-wrap: wrap; margin-top: 12px; font-variant-numeric: tabular-nums; }
.array-title { display: flex; justify-content: space-between; gap: 12px; margin: 18px 0 8px; font-weight: 700; }
.bars { height: 260px; display: flex; align-items: flex-end; gap: 4px; border: 1px solid color-mix(in srgb, CanvasText 18%, transparent); border-radius: 12px; padding: 10px; overflow: hidden; }
.bar { flex: 1; min-width: 8px; background: color-mix(in srgb, CanvasText 55%, transparent); border-radius: 4px 4px 0 0; position: relative; transition: height 120ms linear, background 120ms linear, opacity 120ms linear; }
.bar.empty { opacity: .12; height: 2px !important; }
.bar.compare { background: #d18c00; }
.bar.swap { background: #c0362c; }
.bar.copy-src { background: #2374ab; }
.bar.copy-dst { background: #2e8b57; }
.bar::after { content: attr(data-i); position: absolute; bottom: -22px; left: 50%; transform: translateX(-50%); font-size: 11px; opacity: .7; }
.values { display: grid; grid-template-columns: repeat(auto-fit, minmax(46px, 1fr)); gap: 4px; margin-top: 26px; font-size: 11px; font-variant-numeric: tabular-nums; opacity: .8; }
.value { overflow: hidden; white-space: nowrap; text-overflow: ellipsis; text-align: center; }
.legend { display: flex; gap: 12px; flex-wrap: wrap; margin-top: 12px; font-size: 13px; }
.chip { display: inline-flex; align-items: center; gap: 6px; }
.swatch { width: 14px; height: 14px; border-radius: 4px; background: color-mix(in srgb, CanvasText 55%, transparent); }
.swatch.compare { background: #d18c00; }
.swatch.swap { background: #c0362c; }
.swatch.src { background: #2374ab; }
.swatch.dst { background: #2e8b57; }
code { background: color-mix(in srgb, CanvasText 10%, transparent); padding: 2px 6px; border-radius: 6px; }
</style>
</head>
<body>
<main>
<h1>Sorting visualizer</h1>
<div class="note">Zobrazuje stav <code>main</code> a <code>aux</code> pole po kazde operaci z logu.</div>
<section class="panel controls">
<label>Algoritmus <select id="algo"></select></label>
<label>Krok <input id="step" type="range" min="0" value="0"></label>
<label>Rychlost <input id="speed" type="range" min="1" max="60" value="12"></label>
<button id="play">Play</button><button id="prev">-1 krok</button><button id="next">+1 krok</button>
</section>
<section class="panel">
<div class="meta"><div><strong>Krok:</strong> <span id="stepText"></span></div><div><strong>Operace:</strong> <span id="opText"></span></div><div><strong>Pocet prvku:</strong> <span id="nText"></span></div></div>
<div class="legend"><span class="chip"><span class="swatch compare"></span>compare</span><span class="chip"><span class="swatch swap"></span>swap</span><span class="chip"><span class="swatch src"></span>copy source</span><span class="chip"><span class="swatch dst"></span>copy destination</span></div>
</section>
<section class="panel">
<div class="array-title"><span>Main pole</span><span id="mainStatus"></span></div><div id="mainBars" class="bars"></div><div id="mainValues" class="values"></div>
<div class="array-title"><span>Aux pole</span><span id="auxStatus"></span></div><div id="auxBars" class="bars"></div><div id="auxValues" class="values"></div>
</section>
</main>
<script>
const DATA = __DATA__;
const algoEl = document.getElementById('algo');
const stepEl = document.getElementById('step');
const speedEl = document.getElementById('speed');
const playEl = document.getElementById('play');
const prevEl = document.getElementById('prev');
const nextEl = document.getElementById('next');
let timer = null;
for (const name of Object.keys(DATA)) { const opt = document.createElement('option'); opt.value = name; opt.textContent = `${name} (${DATA[name].steps} operaci)`; algoEl.appendChild(opt); }
function currentData() { return DATA[algoEl.value]; }
function currentFrame() { return currentData().frames[Number(stepEl.value)]; }
function maxVal() { return Math.max(...currentData().initial.filter(v => v != null)); }
function classesFor(kind, index, frame) {
  const op = frame.op; const a = frame.args || []; const classes = ['bar'];
  if (op === 'compare' && kind === 'main' && (index === a[0] || index === a[1])) classes.push('compare');
  if (op === 'swap' && kind === 'main' && (index === a[0] || index === a[1])) classes.push('swap');
  if (op === 'copy') { const [dir, src, dst] = a; if (dir === 'main_to_aux') { if (kind === 'main' && index === src) classes.push('copy-src'); if (kind === 'aux' && index === dst) classes.push('copy-dst'); } else if (dir === 'aux_to_main') { if (kind === 'aux' && index === src) classes.push('copy-src'); if (kind === 'main' && index === dst) classes.push('copy-dst'); } }
  return classes.join(' ');
}
function renderArray(idBars, idValues, arr, kind, frame) {
  const bars = document.getElementById(idBars); const vals = document.getElementById(idValues); bars.innerHTML = ''; vals.innerHTML = ''; const max = maxVal();
  arr.forEach((v, i) => { const bar = document.createElement('div'); bar.className = classesFor(kind, i, frame) + (v == null ? ' empty' : ''); bar.dataset.i = i; bar.title = `${kind}[${i}] = ${v == null ? 'empty' : v}`; bar.style.height = v == null ? '2px' : `${Math.max(4, (v / max) * 100)}%`; bars.appendChild(bar); const val = document.createElement('div'); val.className = 'value'; val.textContent = v == null ? '.' : String(v); val.title = val.textContent; vals.appendChild(val); });
}
function opLabel(frame) { return frame.op === 'init' ? 'init' : `${frame.op} ${frame.args.join(' ')}`; }
function render() { const data = currentData(); const frame = currentFrame(); document.getElementById('stepText').textContent = `${stepEl.value} / ${data.steps}`; document.getElementById('opText').textContent = opLabel(frame); document.getElementById('nText').textContent = data.n; document.getElementById('mainStatus').textContent = frame.op === 'swap' ? 'po swapu' : ''; document.getElementById('auxStatus').textContent = frame.op === 'copy' ? 'po kopii' : ''; renderArray('mainBars', 'mainValues', frame.main, 'main', frame); renderArray('auxBars', 'auxValues', frame.aux, 'aux', frame); }
function configure() { stepEl.max = currentData().steps; stepEl.value = 0; render(); }
function stop() { if (timer) clearInterval(timer); timer = null; playEl.textContent = 'Play'; }
function play() { stop(); playEl.textContent = 'Pause'; timer = setInterval(() => { const max = Number(stepEl.max); const next = Number(stepEl.value) + 1; if (next > max) { stop(); return; } stepEl.value = next; render(); }, Math.max(20, 1000 / Number(speedEl.value))); }
algoEl.addEventListener('change', () => { stop(); configure(); }); stepEl.addEventListener('input', render); speedEl.addEventListener('input', () => { if (timer) play(); }); playEl.addEventListener('click', () => timer ? stop() : play()); prevEl.addEventListener('click', () => { stop(); stepEl.value = Math.max(0, Number(stepEl.value) - 1); render(); }); nextEl.addEventListener('click', () => { stop(); stepEl.value = Math.min(Number(stepEl.max), Number(stepEl.value) + 1); render(); }); configure();
</script>
</body>
</html>
'''

def parse_log(path):
    lines = [line.strip() for line in path.read_text().splitlines() if line.strip()]
    if not lines:
        raise ValueError(f'{path}: empty log')
    initial = [int(x) for x in lines[0].split()]
    main = initial[:]
    aux = [None] * len(main)
    frames = [{'op': 'init', 'args': [], 'main': main[:], 'aux': aux[:]}]
    for line_no, line in enumerate(lines[1:], start=2):
        parts = line.split()
        op = parts[0]
        if op == 'compare':
            args = [int(parts[1]), int(parts[2])]
        elif op == 'swap':
            args = [int(parts[1]), int(parts[2])]
            i, j = args
            main[i], main[j] = main[j], main[i]
        elif op == 'copy':
            direction = parts[1]
            src = int(parts[2])
            dst = int(parts[3])
            if direction == 'main_to_aux':
                aux[dst] = main[src]
            elif direction == 'aux_to_main':
                main[dst] = aux[src]
            else:
                raise ValueError(f'{path}:{line_no}: unknown copy direction {direction}')
            args = [direction, src, dst]
        else:
            raise ValueError(f'{path}:{line_no}: unknown operation {op}')
        frames.append({'op': op, 'args': args, 'line': line_no, 'main': main[:], 'aux': aux[:]})
    return {'initial': initial, 'frames': frames, 'steps': len(frames) - 1, 'n': len(initial)}

def main():
    parser = argparse.ArgumentParser(description='Create a standalone HTML visualization for sorting logs.')
    parser.add_argument('logs', nargs='+', help='Log files, for example insert.log merge.log')
    parser.add_argument('-o', '--output', default='sorting_visualizer.html', help='Output HTML file')
    args = parser.parse_args()
    data = {}
    for log in args.logs:
        path = Path(log)
        name = path.stem
        data[name] = parse_log(path)
    Path(args.output).write_text(HTML_TEMPLATE.replace('__DATA__', json.dumps(data)))
    print(args.output)

if __name__ == '__main__':
    main()

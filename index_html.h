#pragma once
#include <Arduino.h>

// واجهة الويب (في ملف منفصل عشان Arduino preprocessor بيلخبط مع الـ raw string الطويلة)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html dir="rtl" lang="ar">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="theme-color" content="#0f172a">
<title>Voice Hub</title>
<style>
:root{--bg:#f1f5f9;--card:#fff;--text:#0f172a;--mute:#64748b;--line:#e2e8f0;--pri:#2563eb;--pri2:#1d4ed8;--ok:#16a34a;--bad:#dc2626;--chip:#eef2ff;--term:#0b1220;--termtx:#cbd5e1}
@media(prefers-color-scheme:dark){:root{--bg:#0b1220;--card:#131c2e;--text:#e5e7eb;--mute:#94a3b8;--line:#22304a;--pri:#3b82f6;--pri2:#2563eb;--chip:#1b2740;--term:#060b16}}
*{box-sizing:border-box}
[hidden]{display:none!important}
body{margin:0;background:var(--bg);color:var(--text);font-family:'Segoe UI',Tahoma,'Noto Sans Arabic',sans-serif;font-size:15px}
.wrap{max-width:720px;margin:0 auto;padding:16px}
header{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-bottom:14px}
header h1{font-size:20px;margin:0}
.live{display:flex;align-items:center;gap:6px;font-size:12px;color:var(--mute)}
.dot{width:9px;height:9px;border-radius:50%;background:var(--mute)}
.dot.on{background:var(--ok);box-shadow:0 0 0 3px rgba(22,163,74,.2)}
.tabs{display:flex;gap:6px;background:var(--card);padding:5px;border-radius:12px;border:1px solid var(--line);margin-bottom:14px}
.tab{flex:1;border:0;background:none;color:var(--mute);padding:10px;border-radius:8px;font:inherit;font-weight:600;cursor:pointer}
.tab.active{background:var(--pri);color:#fff}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px;margin-bottom:14px}
.card h2{font-size:16px;margin:0 0 12px;display:flex;align-items:center;justify-content:space-between}
.badge{font-size:12px;background:var(--chip);color:var(--pri);padding:2px 9px;border-radius:99px;font-weight:700}
input,select{width:100%;padding:11px 12px;margin-bottom:10px;border:1px solid var(--line);border-radius:9px;background:var(--bg);color:var(--text);font:inherit}
input:focus,select:focus{outline:2px solid var(--pri);outline-offset:-1px}
.btn{border:0;border-radius:9px;padding:10px 14px;font:inherit;font-weight:600;cursor:pointer;background:var(--pri);color:#fff;transition:.15s}
.btn:hover{background:var(--pri2)}
.btn:disabled{opacity:.6;cursor:wait}
.btn.full{width:100%}
.btn.ghost{background:var(--chip);color:var(--text)}
.btn.ghost:hover{background:var(--line)}
.btn.run{background:var(--ok)}
.btn.del{background:none;color:var(--bad);border:1px solid var(--bad)}
.btn.del:hover{background:var(--bad);color:#fff}
.btn.danger{background:var(--bad)}
.cmd{display:flex;align-items:center;justify-content:space-between;gap:10px;padding:12px;border:1px solid var(--line);border-radius:11px;margin-bottom:9px}
.cmd-main{min-width:0;flex:1}
.cmd-name{font-weight:700;margin-bottom:5px;word-break:break-word}
.cmd-meta{display:flex;align-items:center;gap:8px;min-width:0}
.cmd-meta code{direction:ltr;font-size:12px;color:var(--mute);overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.cmd-actions{display:flex;gap:6px;flex-shrink:0}
.empty{text-align:center;color:var(--mute);padding:22px 0}
.info{display:grid;grid-template-columns:repeat(auto-fit,minmax(130px,1fr));gap:8px}
.chip{background:var(--chip);border-radius:10px;padding:9px 11px}
.chip small{display:block;color:var(--mute);font-size:11px}
.chip b{direction:ltr;display:inline-block;font-size:14px}
.tools{display:flex;flex-wrap:wrap;gap:6px;margin-bottom:10px;align-items:center}
.tools .btn{padding:7px 11px;font-size:13px}
.tools label{font-size:13px;color:var(--mute);display:flex;align-items:center;gap:5px;margin-inline-start:auto}
.tools input{width:auto;margin:0}
#term{direction:ltr;text-align:left;background:var(--term);color:var(--termtx);border-radius:10px;padding:10px;height:340px;overflow-y:auto;font:12.5px/1.55 ui-monospace,Consolas,Menlo,monospace}
#term div{white-space:pre-wrap;word-break:break-all}
#term .t{color:#475569;margin-right:8px}
#term .err{color:#f87171}#term .ok{color:#4ade80}#term .wake{color:#22d3ee}#term .proc{color:#c084fc}#term .sys{color:#fbbf24}
.send{display:flex;gap:6px;margin-top:10px}
.send input{margin:0}
.send .btn{white-space:nowrap}
textarea{width:100%;padding:11px 12px;margin-bottom:10px;border:1px solid var(--line);border-radius:9px;background:var(--bg);color:var(--text);font:inherit;resize:vertical}
textarea:focus{outline:2px solid var(--pri);outline-offset:-1px}
.lbl{display:block;font-size:13px;color:var(--mute);margin:2px 0 6px}
.step{display:flex;gap:6px;align-items:flex-start}
.step select{width:90px;flex-shrink:0}
.step .btn{padding:10px 12px}
.phrases{display:flex;flex-wrap:wrap;gap:5px;margin-bottom:6px}
.ph{font-size:12px;background:var(--chip);padding:2px 8px;border-radius:99px}
.step select.irsel{width:auto;flex:1;min-width:0}
.irstat{margin:10px 0;color:var(--mute);font-size:14px}
.irrow{display:flex;gap:8px}.irrow>*{flex:1}
.step .sv{display:flex;gap:6px;flex:1;min-width:0}.step .sv select{width:auto;flex:1;min-width:0}
.step .dl{display:flex;gap:6px;flex:1;min-width:0;align-items:center}.step .dl span{color:var(--mute);font-size:13px;white-space:nowrap;margin-bottom:10px}
.sgrid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.so{border:1px solid var(--line);border-radius:12px;padding:12px;background:var(--bg)}
.so.on{border-color:var(--ok);box-shadow:0 0 0 2px rgba(22,163,74,.15)}
.so h3{margin:0 0 6px;font-size:14px;display:flex;justify-content:space-between;align-items:center}
.so .m{font-size:12px;color:var(--mute);margin-bottom:8px;direction:ltr;text-align:left}
.so .btn{width:100%;padding:8px}
.so.bad{border-color:var(--bad)}
.sinfo{font-size:13px;color:var(--mute);line-height:1.8;direction:ltr;text-align:left}
#toast{position:fixed;bottom:22px;left:50%;transform:translate(-50%,30px);background:#0f172a;color:#fff;padding:10px 18px;border-radius:99px;opacity:0;pointer-events:none;transition:.25s;font-size:14px;z-index:9}
#toast.show{opacity:1;transform:translate(-50%,0)}
#toast.bad{background:var(--bad)}
.step .mv{display:flex;flex-direction:column;gap:2px;flex-shrink:0}
.step .mv .btn{padding:0 8px;height:20px;line-height:1;font-size:11px;margin:0}
.step .grip{cursor:grab;color:var(--mute);font-size:18px;padding:10px 4px;user-select:none;touch-action:none}
.step.dragging{opacity:.4}
.step.over{outline:2px dashed var(--pri);outline-offset:2px;border-radius:9px}
.bk{display:flex;gap:8px;margin-bottom:10px}.bk>*{flex:1}
</style>
</head>
<body>
<div class="wrap">
  <header>
    <h1>🎙️ Voice Hub</h1>
    <div class="live"><span class="dot" id="dot"></span><span id="ip">--</span></div>
  </header>

  <div class="tabs">
    <button class="tab active" data-tab="cmds" onclick="showTab('cmds')">الأوامر</button>
    <button class="tab" data-tab="ir" onclick="showTab('ir')">IR</button>
    <button class="tab" data-tab="strip" onclick="showTab('strip')">المشترك</button>
    <button class="tab" data-tab="maint" onclick="showTab('maint')">الصيانة</button>
  </div>

  <section class="panel" id="tab-cmds">
    <div class="card" id="form-card">
      <h2><span id="form-title">إضافة أمر جديد</span><button class="btn ghost" id="cancelBtn" onclick="resetForm()" hidden>إلغاء التعديل</button></h2>
      <input type="text" id="name" placeholder="اسم الأمر (مثال: نور السقف)">
      <input type="text" id="cat" list="cats" placeholder="التصنيف (مثال: إضاءة) - اختياري">
      <datalist id="cats"></datalist>
      <label class="lbl">الجمل المنطوقة (جملة في كل سطر - كلها بتنفذ نفس الأمر)</label>
      <textarea id="phrases" rows="3" dir="auto" placeholder="شغل نور السقف&#10;شغلي نور السقف&#10;turn on ceiling light"></textarea>
      <label class="lbl">الخطوات (بتتنفذ بالترتيب)</label>
      <div id="steps"></div>
      <button class="btn ghost full" onclick="addStep()">＋ إضافة خطوة</button>
      <button class="btn full" style="margin-top:10px" onclick="saveCommand()">حفظ الأمر</button>
    </div>

    <div class="card">
      <h2 id="heardHead" style="cursor:pointer;user-select:none" onclick="toggleHeard()"><span>آخر ما سمعه الجهاز</span><span id="heardArrow">◂</span></h2>
      <div id="heardBody" hidden>
        <div class="tools"><button class="btn ghost" onclick="loadHeard()">تحديث</button> <button class="btn ghost" onclick="clearHeard()">🧹 مسح</button><label><input type="checkbox" id="heardOn" onchange="setHeardOn()"> تسجيل ما يسمعه الجهاز (وطباعته في السيريال)</label></div>
        <div class="irstat" style="margin-top:0">لو سمع كلمة غلط، اضغط ＋ تضيفها كجملة للأمر (بتتحط في خانة الجمل فوق، اختار الأمر واحفظ).</div>
        <div id="heard"></div>
      </div>
    </div>

    <div id="groups"></div>

    <div class="card">
      <h2>كل الأوامر <span class="badge" id="cmd-count">0</span></h2>
      <button class="btn danger full" onclick="clearAllCommands()">مسح كل الأوامر</button>
    </div>
  </section>

  <section class="panel" id="tab-strip" hidden>
    <div class="card">
      <h2>حالة المشترك <span class="badge" id="st-badge">غير متصل</span></h2>
      <div class="sinfo" id="st-info">--</div>
      <div class="sinfo" style="margin-top:8px">لإضافة/إعادة ربط المشترك: من تطبيق MTTL Control &gt; Device setup اكتب IP الجهاز ده: <b id="st-ip">--</b> (بورت 10086)</div>
    </div>
    <div class="card">
      <h2>المخارج</h2>
      <div class="sgrid" id="st-grid"></div>
      <div class="irrow" style="margin-top:10px">
        <button class="btn run" onclick="stripSet(0,'on')">تشغيل الكل</button>
        <button class="btn danger" onclick="stripSet(0,'off')">إطفاء الكل</button>
      </div>
      <button class="btn ghost full" style="margin-top:10px" onclick="stripRefresh()">تحديث القراءات</button>
    </div>
  </section>

  <section class="panel" id="tab-ir" hidden>
    <div class="card">
      <h2>أطراف الـ IR (GPIO)</h2>
      <label class="lbl">المرسل (IR LED) - TX</label>
      <select id="irtx"></select>
      <label class="lbl">المستقبل (VS1838B) - RX</label>
      <select id="irrx"></select>
      <button class="btn full" onclick="saveIrCfg()">حفظ الأطراف</button>
    </div>

    <div class="card">
      <h2>تعلّم إشارة جديدة</h2>
      <button class="btn full" id="learnBtn" onclick="startLearn()">🎯 ابدأ التعلم</button>
      <div class="irstat" id="irstat"></div>
      <div id="irnew" hidden>
        <input type="text" id="irname" placeholder="اسم الإشارة (مثال: تشغيل البروجيكتور)">
        <div class="irrow">
          <select id="irkhz"><option value="38">38 kHz</option><option value="36">36 kHz</option><option value="40">40 kHz</option><option value="56">56 kHz</option></select>
          <select id="irrep"><option value="1">إرسال مرة</option><option value="2">مرتين</option><option value="3">3 مرات</option></select>
        </div>
        <div class="irrow">
          <button class="btn ghost" onclick="testIr()">▶ جرّب</button>
          <button class="btn" onclick="saveIr()">حفظ</button>
        </div>
      </div>
    </div>

    <div class="card">
      <h2>الإشارات المحفوظة <span class="badge" id="ir-count">0</span></h2>
      <div id="irlist"></div>
    </div>
  </section>

  <section class="panel" id="tab-maint" hidden>
    <div class="card">
      <h2>حالة الجهاز</h2>
      <div class="info" id="info"></div>
    </div>

    <div class="card">
      <h2>الميكروفون</h2>
      <label class="lbl">تكبير كلمة التنبيه (wake word) - زوّده لو مش بيسمعك من بعيد (2 - 128)</label>
      <input type="number" id="micg" min="2" max="128" step="2" dir="ltr">
      <label class="lbl">أقل مستوى يعتبر كلام أثناء التسجيل - قلله لو بيفوّت كلامك الواطي (2 - 60)</label>
      <input type="number" id="micv" min="2" max="60" step="1" dir="ltr">
      <button class="btn full" onclick="saveMic()">حفظ</button>
      <div class="sinfo" id="micinfo" style="margin-top:10px">--</div>
    </div>

    <div class="card">
      <h2>Serial Monitor</h2>
      <div class="tools">
        <button class="btn ghost" id="pauseBtn" onclick="togglePause()">⏸ إيقاف</button>
        <button class="btn ghost" onclick="clearTerm()">🧹 مسح</button>
        <button class="btn ghost" onclick="copyLog()">📋 نسخ</button>
        <button class="btn ghost" onclick="downloadLog()">⬇ تحميل</button>
        <label><input type="checkbox" id="autoscroll" checked> تمرير تلقائي</label>
      </div>
      <div id="term"></div>
      <div class="send">
        <input type="text" id="say" dir="auto" placeholder="جرّب أمر نصي بدل الكلام (مثال: turn on ring light)" onkeydown="if(event.key==='Enter')sendSay()">
        <button class="btn" onclick="sendSay()">إرسال</button>
      </div>
    </div>

    <div class="card">
      <h2>نسخة احتياطية للأوامر</h2>
      <div class="bk">
        <button class="btn ghost" onclick="exportCmds()">⬇ تصدير (JSON)</button>
        <button class="btn ghost" onclick="$('#impFile').click()">⬆ استيراد (JSON)</button>
      </div>
      <input type="file" id="impFile" accept=".json,application/json" hidden onchange="importCmds(this)">
      <div class="sinfo">التصدير بيحفظ كل الأوامر (الجمل + الخطوات + الترتيب) في ملف على جهازك. وبيحفظ كمان الإشارات الـ IR المتعلَّمة، وبيتعاد ربطها بالأوامر تلقائي وقت الاستيراد.</div>
    </div>

    <div class="card">
      <h2>أدوات</h2>
      <button class="btn danger full" onclick="reboot()">🔄 إعادة تشغيل الجهاز</button>
    </div>
  </section>
</div>
<div id="toast"></div>

<script>
const $ = s => document.querySelector(s);
const el = (tag, cls, txt) => { const e = document.createElement(tag); if (cls) e.className = cls; if (txt !== undefined) e.textContent = txt; return e; };
const post = (url, data) => fetch(url, {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body:new URLSearchParams(data||{}).toString()});
let toastT;
function toast(msg, bad) {
  const t = $('#toast'); t.textContent = msg; t.className = 'show' + (bad ? ' bad' : '');
  clearTimeout(toastT); toastT = setTimeout(() => t.className = '', 2400);
}

/* ---------- tabs ---------- */
function showTab(name) {
  document.querySelectorAll('.tab').forEach(b => b.classList.toggle('active', b.dataset.tab === name));
  document.querySelectorAll('.panel').forEach(p => p.hidden = p.id !== 'tab-' + name);
  if (name === 'ir') { loadIrCfg(); loadIrList(); }
  if (name === 'strip') startStrip(); else stopStrip();
  if (name === 'cmds' && !$('#heardBody').hidden) loadHeard();
  if (name === 'maint') loadMic();
  if (name === 'maint') { startLog(); loadInfo(); } else stopLog();
}

/* ---------- commands ---------- */
let cmds = [], editId = 0, irList = [], learnT = null;
const PH = {http: 'http://192.168.1.36/TOGGLE', wol: 'D8:5E:D3:C2:A1:F8', relay: '1:on  |  1:off  |  2:toggle', delay: '5'};

function fillIrSel(sel, val) {
  const cur = val !== undefined ? String(val) : sel.value;
  sel.textContent = '';
  irList.forEach(i => { const o = el('option', '', i.name); o.value = i.id; sel.appendChild(o); });
  if (!irList.length) { const o = el('option', '', 'لا توجد إشارات - تعلّم من تبويب IR'); o.value = ''; sel.appendChild(o); }
  sel.value = cur;
}
function refreshIrSelects() { document.querySelectorAll('.irsel').forEach(s => fillIrSel(s)); }

function addStep(type, val) {
  const row = el('div', 'step');
  const sel = el('select', 'tsel');
  [['http', 'HTTP'], ['wol', 'WOL'], ['ir', 'IR'], ['strip', 'مشترك'], ['delay', 'تأخير'], ['relay', 'RELAY']].forEach(([v, t]) => { const o = el('option', '', t); o.value = v; sel.appendChild(o); });
  sel.value = type || 'http';
  const inp = el('input', 'tin'); inp.type = 'text'; inp.dir = 'ltr';
  const irs = el('select', 'irsel'); fillIrSel(irs, type === 'ir' ? val : undefined);
  // مشترك: مخرج + حالة
  const sv = el('div', 'sv');
  const sn = el('select', 'sn'); [['0', 'كل المخارج'], ['1', 'مخرج 1'], ['2', 'مخرج 2'], ['3', 'مخرج 3'], ['4', 'مخرج 4']].forEach(([v, t]) => { const o = el('option', '', t); o.value = v; sn.appendChild(o); });
  const sa = el('select', 'sa'); [['on', 'تشغيل'], ['off', 'إطفاء'], ['toggle', 'تبديل']].forEach(([v, t]) => { const o = el('option', '', t); o.value = v; sa.appendChild(o); });
  sv.appendChild(sn); sv.appendChild(sa);
  // تأخير: ثواني
  const dl = el('div', 'dl');
  const din = el('input', 'din'); din.type = 'number'; din.min = '0.1'; din.max = '3600'; din.step = '0.5'; din.dir = 'ltr'; din.placeholder = '5';
  dl.appendChild(din); dl.appendChild(el('span', '', 'ثانية'));
  if (type === 'strip' && val) { const [n, a] = String(val).split(':'); sn.value = n; sa.value = a || 'on'; }
  else if (type === 'delay') din.value = val || '';
  else if (type !== 'ir') inp.value = val || '';
  const upd = () => {
    const t = sel.value;
    inp.hidden = !(t === 'http' || t === 'wol' || t === 'relay');
    irs.hidden = t !== 'ir'; sv.hidden = t !== 'strip'; dl.hidden = t !== 'delay';
    if (!inp.hidden) inp.placeholder = PH[t];
  };
  sel.onchange = upd; upd();
  const rm = el('button', 'btn del', '✕');
  rm.onclick = () => { if ($('#steps').children.length > 1) row.remove(); };
  const grip = el('span', 'grip', '⠿'); grip.title = 'اسحب للترتيب';
  const mv = el('div', 'mv');
  const up = el('button', 'btn ghost', '▲'); up.onclick = () => { const p = row.previousElementSibling; if (p) row.parentNode.insertBefore(row, p); };
  const dn = el('button', 'btn ghost', '▼'); dn.onclick = () => { const n = row.nextElementSibling; if (n) row.parentNode.insertBefore(n, row); };
  mv.appendChild(up); mv.appendChild(dn);
  [grip, mv, sel, inp, irs, sv, dl, rm].forEach(x => row.appendChild(x));
  setupDrag(row, grip);
  $('#steps').appendChild(row);
}
/* سحب وإفلات الخطوات (ماوس + لمس) */
let dragRow = null;
function setupDrag(row, grip) {
  grip.onpointerdown = e => {
    e.preventDefault(); dragRow = row; row.classList.add('dragging');
    grip.setPointerCapture(e.pointerId);
  };
  grip.onpointermove = e => {
    if (dragRow !== row) return;
    const t = document.elementFromPoint(e.clientX, e.clientY);
    const over = t && t.closest ? t.closest('#steps .step') : null;
    document.querySelectorAll('#steps .step.over').forEach(x => x.classList.remove('over'));
    if (over && over !== row) over.classList.add('over');
  };
  const end = e => {
    if (dragRow !== row) return;
    const t = document.elementFromPoint(e.clientX, e.clientY);
    const over = t && t.closest ? t.closest('#steps .step') : null;
    if (over && over !== row) {
      const rows = [...$('#steps').children];
      if (rows.indexOf(row) < rows.indexOf(over)) over.after(row); else over.before(row);
    }
    document.querySelectorAll('#steps .step').forEach(x => x.classList.remove('over', 'dragging'));
    dragRow = null;
  };
  grip.onpointerup = end; grip.onpointercancel = end;
}
function stepVal(r) {
  const t = r.querySelector('.tsel').value;
  if (t === 'ir') return r.querySelector('.irsel').value;
  if (t === 'strip') return r.querySelector('.sn').value + ':' + r.querySelector('.sa').value;
  if (t === 'delay') return r.querySelector('.din').value;
  return r.querySelector('.tin').value;
}
function stepLabel(s) {
  if (s.type === 'ir') return 'IR: ' + ((irList.find(i => String(i.id) === String(s.val)) || {}).name || '؟');
  if (s.type === 'delay') return '⏱ ' + s.val + ' ث';
  if (s.type === 'strip') { const [n, a] = String(s.val).split(':'); return 'مشترك ' + (n === '0' ? 'الكل' : n) + ': ' + ({on: 'تشغيل', off: 'إطفاء', toggle: 'تبديل'}[a] || a); }
  return s.val;
}

function resetForm() {
  editId = 0;
  $('#name').value = ''; $('#cat').value = ''; $('#phrases').value = '';
  $('#steps').textContent = ''; addStep();
  $('#form-title').textContent = 'إضافة أمر جديد';
  $('#cancelBtn').hidden = true;
}

function editCmd(id) {
  const c = cmds.find(x => x.id === id); if (!c) return;
  editId = id;
  $('#name').value = c.name; $('#cat').value = c.cat || '';
  $('#phrases').value = (c.phrases || []).join('\n');
  $('#steps').textContent = '';
  (c.steps || []).forEach(s => addStep(s.type, s.val));
  if (!$('#steps').children.length) addStep();
  $('#form-title').textContent = 'تعديل: ' + c.name;
  $('#cancelBtn').hidden = false;
  window.scrollTo({top: 0, behavior: 'smooth'});
}

async function loadCommands() {
  try {
    const r = await (await fetch('/api/commands', {cache: 'no-store'})).json();
    cmds = Array.isArray(r) ? r : [];
    renderCommands();
  } catch (e) { toast('تعذر تحميل الأوامر', true); }
}

function renderCommands() {
  const box = $('#groups'); box.textContent = '';
  $('#cmd-count').textContent = cmds.length;
  const cats = [...new Set(cmds.map(c => c.cat || 'عام'))];
  const dl = $('#cats'); dl.textContent = '';
  cats.forEach(c => { const o = el('option'); o.value = c; dl.appendChild(o); });
  if (!cmds.length) {
    const card = el('div', 'card'); card.appendChild(el('div', 'empty', 'لا توجد أوامر محفوظة بعد')); box.appendChild(card); return;
  }
  cats.forEach(cat => {
    const list = cmds.filter(c => (c.cat || 'عام') === cat);
    const card = el('div', 'card');
    const h = el('h2'); h.appendChild(el('span', '', cat)); h.appendChild(el('span', 'badge', list.length));
    card.appendChild(h);
    list.forEach(c => {
      const row = el('div', 'cmd');
      const main = el('div', 'cmd-main');
      main.appendChild(el('div', 'cmd-name', c.name));
      const ph = el('div', 'phrases');
      (c.phrases || []).forEach(p => ph.appendChild(el('span', 'ph', p)));
      main.appendChild(ph);
      const meta = el('div', 'cmd-meta');
      meta.appendChild(el('span', 'badge', (c.steps || []).length + ' خطوة'));
      const first = (c.steps || [])[0];
      if (first) {
        const fv = stepLabel(first);
        meta.appendChild(el('code', '', fv + ((c.steps.length > 1) ? ' …' : '')));
      }
      main.appendChild(meta);
      const acts = el('div', 'cmd-actions');
      const run = el('button', 'btn run', '▶'); run.onclick = () => runCmd(c.id, c.name, run);
      const ed = el('button', 'btn ghost', '✎'); ed.onclick = () => editCmd(c.id);
      const del = el('button', 'btn del', '🗑'); del.onclick = () => delCmd(c.id, c.name);
      acts.appendChild(run); acts.appendChild(ed); acts.appendChild(del);
      row.appendChild(main); row.appendChild(acts);
      card.appendChild(row);
    });
    box.appendChild(card);
  });
}

async function saveCommand() {
  const name = $('#name').value.trim();
  const cat = $('#cat').value.trim() || 'عام';
  let phrases = $('#phrases').value.split('\n').map(s => s.trim().toLowerCase()).filter(Boolean);
  if (!phrases.length && name) phrases = [name.toLowerCase()];
  const steps = [...$('#steps').children].map(r => ({type: r.querySelector('.tsel').value, val: String(stepVal(r)).trim()})).filter(s => s.val);
  if (!name || !phrases.length || !steps.length) { toast('أكمل البيانات المطلوبة', true); return; }
  try {
    const r = await fetch('/api/save', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({id: editId, name, cat, phrases, steps})});
    if (!r.ok) throw 0;
    toast(editId ? 'تم تحديث الأمر' : 'تم حفظ الأمر');
    resetForm(); loadCommands();
  } catch (e) { toast('فشل الحفظ (المساحة ممتلئة؟)', true); }
}

async function runCmd(id, name, btn) {
  const old = btn.textContent; btn.disabled = true; btn.textContent = '...';
  try {
    const r = await post('/api/run', {id});
    toast(r.ok ? 'تم إرسال: ' + name : 'فشل التنفيذ: ' + name, !r.ok);
  } catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
  btn.disabled = false; btn.textContent = old;
}

async function delCmd(id, name) {
  if (!confirm('حذف الأمر "' + name + '"؟')) return;
  try {
    const r = await post('/api/delete', {id});
    if (!r.ok) throw 0;
    if (editId === id) resetForm();
    toast('تم الحذف'); loadCommands();
  } catch (e) { toast('فشل الحذف', true); }
}

async function clearAllCommands() {
  if (!confirm('هل أنت متأكد من مسح جميع الأوامر؟')) return;
  await post('/api/clear'); resetForm(); toast('تم مسح كل الأوامر'); loadCommands();
}

/* ---------- ما سمعه الجهاز + الميكروفون ---------- */
async function loadHeard() {
  try {
    const a = await (await fetch('/api/heard', {cache: 'no-store'})).json();
    const box = $('#heard'); box.textContent = '';
    if (!a.length) { box.appendChild(el('div', 'empty', $('#heardOn').checked ? 'لسه ما سمعش حاجة' : 'التسجيل متوقف')); return; }
    a.forEach(h => {
      const row = el('div', 'cmd');
      const main = el('div', 'cmd-main'); main.appendChild(el('div', 'cmd-name', (h.ok ? '✔ ' : '✖ ') + h.t));
      const acts = el('div', 'cmd-actions');
      const add = el('button', 'btn ghost', '＋');
      add.onclick = () => { const ta = $('#phrases'); ta.value = (ta.value ? ta.value.replace(/\n*$/, '') + '\n' : '') + h.t; toast('اتضافت لخانة الجمل'); window.scrollTo({top: 0, behavior: 'smooth'}); };
      acts.appendChild(add); row.appendChild(main); row.appendChild(acts); box.appendChild(row);
    });
  } catch (e) {}
}
function toggleHeard(force) {
  const open = force !== undefined ? force : $('#heardBody').hidden;
  $('#heardBody').hidden = !open;
  $('#heardArrow').textContent = open ? '▾' : '◂';
  $('#heardHead').style.marginBottom = open ? '' : '0';
  try { localStorage.setItem('heardOpen', open ? '1' : '0'); } catch (e) {}
  if (open) loadHeardCfg().then(loadHeard);
}
async function clearHeard() { try { await post('/api/heard/clear'); toast('تم المسح'); loadHeard(); } catch (e) { toast('لا يوجد اتصال بالجهاز', true); } }
async function setHeardOn() {
  const on = $('#heardOn').checked;
  try { await post('/api/heard/cfg', {on: on ? 1 : 0}); toast(on ? 'التسجيل شغّال' : 'التسجيل متوقف'); loadHeard(); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}
async function loadHeardCfg() { try { const c = await (await fetch('/api/heard/cfg', {cache: 'no-store'})).json(); $('#heardOn').checked = !!c.on; } catch (e) {} }
async function loadMic() {
  try {
    const m = await (await fetch('/api/mic', {cache: 'no-store'})).json();
    if (document.activeElement !== $('#micg')) $('#micg').value = m.g;
    if (document.activeElement !== $('#micv')) $('#micv').value = m.v;
    $('#micinfo').textContent = 'ضوضاء الغرفة: ' + m.n + ' | آخر تسجيل: قمة ' + m.pk + ' · تضخيم ' + m.gain + 'x · كلام ' + m.ms + ' ms';
  } catch (e) {}
}
async function saveMic() {
  try { const r = await post('/api/mic', {g: $('#micg').value, v: $('#micv').value}); toast(r.ok ? 'تم الحفظ' : 'قيمة غير صالحة', !r.ok); loadMic(); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}
setInterval(() => { if (!$('#tab-maint').hidden) loadMic(); if (!$('#tab-cmds').hidden && !$('#heardBody').hidden) loadHeard(); }, 2500);

/* ---------- المشترك ---------- */
let stripT = null;
function startStrip() { stripStatus(); clearInterval(stripT); stripT = setInterval(stripStatus, 2000); }
function stopStrip() { clearInterval(stripT); stripT = null; }
async function stripStatus() {
  try {
    const s = await (await fetch('/api/strip/status', {cache: 'no-store'})).json();
    $('#st-ip').textContent = s.ip;
    $('#st-badge').textContent = s.on ? 'متصل' : 'غير متصل';
    $('#st-info').textContent = s.on ? ('MAC ' + s.mac + ' | FW ' + s.fw + ' | آخر قراءة: ' + (s.age < 0 ? '--' : s.age + ' ث')) : 'مستني اتصال المشترك على ' + s.ip + ':' + s.port;
    const g = $('#st-grid'); g.textContent = '';
    s.o.forEach((o, i) => {
      const c = el('div', 'so' + (o.on ? ' on' : '') + (o.ok ? '' : ' bad'));
      const h = el('h3'); h.appendChild(el('span', '', 'مخرج ' + (i + 1))); h.appendChild(el('span', 'badge', o.on ? 'ON' : 'OFF'));
      c.appendChild(h);
      c.appendChild(el('div', 'm', o.w.toFixed(1) + ' W | ' + (o.wh / 1000).toFixed(3) + ' kWh | ' + o.t + '°C' + (o.ok ? '' : ' ⚠')));
      const b = el('button', 'btn ' + (o.on ? 'danger' : 'run'), o.on ? 'إطفاء' : 'تشغيل');
      b.disabled = !s.on; b.onclick = () => stripSet(i + 1, o.on ? 'off' : 'on');
      c.appendChild(b); g.appendChild(c);
    });
  } catch (e) {}
}
async function stripSet(n, st) {
  try { const r = await post('/api/strip/set', {n, s: st}); toast(r.ok ? 'تم الإرسال' : (r.status === 409 ? 'المشترك غير متصل' : 'فشل'), !r.ok); setTimeout(stripStatus, 900); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}
async function stripRefresh() { try { await post('/api/strip/refresh'); setTimeout(stripStatus, 700); } catch (e) {} }

/* ---------- IR ---------- */
const IR_PINS = [1,2,4,6,9,10,11,12,13,14,18,21,38,39,40,41,42,47];
function fillPins() {
  ['#irtx', '#irrx'].forEach(s => { const sel = $(s); sel.textContent = ''; IR_PINS.forEach(p => { const o = el('option', '', 'GPIO ' + p); o.value = p; sel.appendChild(o); }); });
}
async function loadIrCfg() {
  try { const c = await (await fetch('/api/ir/cfg', {cache: 'no-store'})).json(); $('#irtx').value = c.tx; $('#irrx').value = c.rx; } catch (e) {}
}
async function saveIrCfg() {
  const tx = $('#irtx').value, rx = $('#irrx').value;
  if (tx === rx) { toast('لازم الطرفين يكونوا مختلفين', true); return; }
  try { const r = await post('/api/ir/cfg', {tx, rx}); if (!r.ok) throw 0; toast('تم حفظ الأطراف'); }
  catch (e) { toast('فشل حفظ الأطراف', true); }
}
async function loadIrList() {
  try { const r = await (await fetch('/api/ir/list', {cache: 'no-store'})).json(); irList = Array.isArray(r) ? r : []; } catch (e) { irList = []; }
  renderIr(); refreshIrSelects(); if (cmds.length) renderCommands();
}
function renderIr() {
  const box = $('#irlist'); box.textContent = '';
  $('#ir-count').textContent = irList.length;
  if (!irList.length) { box.appendChild(el('div', 'empty', 'لا توجد إشارات محفوظة بعد')); return; }
  irList.forEach(c => {
    const row = el('div', 'cmd');
    const main = el('div', 'cmd-main');
    main.appendChild(el('div', 'cmd-name', c.name));
    const meta = el('div', 'cmd-meta');
    meta.appendChild(el('span', 'badge', c.len + ' نبضة'));
    meta.appendChild(el('code', '', c.khz + ' kHz × ' + c.rep));
    main.appendChild(meta);
    const acts = el('div', 'cmd-actions');
    const run = el('button', 'btn run', '▶'); run.onclick = () => sendIr(c.id, run);
    const ed = el('button', 'btn ghost', '✎'); ed.onclick = () => renameIr(c);
    const del = el('button', 'btn del', '🗑'); del.onclick = () => delIr(c);
    acts.appendChild(run); acts.appendChild(ed); acts.appendChild(del);
    row.appendChild(main); row.appendChild(acts); box.appendChild(row);
  });
}
async function startLearn() {
  try { await post('/api/ir/learn'); } catch (e) { toast('لا يوجد اتصال بالجهاز', true); return; }
  $('#irnew').hidden = true;
  $('#irstat').textContent = 'وجّه الريموت ناحية المستقبل واضغط الزر...';
  clearInterval(learnT); learnT = setInterval(pollLearn, 500);
}
async function pollLearn() {
  try {
    const s = await (await fetch('/api/ir/status', {cache: 'no-store'})).json();
    if (s.state === 'captured') { clearInterval(learnT); $('#irstat').textContent = 'تم الالتقاط: ' + s.len + ' نبضة'; $('#irnew').hidden = false; }
    else if (s.state === 'timeout') { clearInterval(learnT); $('#irstat').textContent = 'انتهى الوقت - جرّب تاني'; }
  } catch (e) {}
}
async function testIr() {
  try { const r = await post('/api/ir/send', {pending: 1, khz: $('#irkhz').value, rep: $('#irrep').value}); toast(r.ok ? 'تم الإرسال' : 'فشل الإرسال', !r.ok); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
}
async function saveIr() {
  const name = $('#irname').value.trim();
  if (!name) { toast('اكتب اسم للإشارة', true); return; }
  try {
    const r = await post('/api/ir/save', {id: 0, name, khz: $('#irkhz').value, rep: $('#irrep').value});
    if (!r.ok) throw 0;
    $('#irname').value = ''; $('#irnew').hidden = true; $('#irstat').textContent = '';
    toast('تم حفظ الإشارة'); loadIrList();
  } catch (e) { toast('فشل الحفظ', true); }
}
async function sendIr(id, btn) {
  const old = btn.textContent; btn.disabled = true; btn.textContent = '...';
  try { const r = await post('/api/ir/send', {id}); toast(r.ok ? 'تم الإرسال' : 'فشل الإرسال', !r.ok); }
  catch (e) { toast('لا يوجد اتصال بالجهاز', true); }
  btn.disabled = false; btn.textContent = old;
}
async function renameIr(c) {
  const name = prompt('اسم الإشارة:', c.name); if (!name || !name.trim()) return;
  try { const r = await post('/api/ir/save', {id: c.id, name: name.trim(), khz: c.khz, rep: c.rep}); if (!r.ok) throw 0; toast('تم التعديل'); loadIrList(); }
  catch (e) { toast('فشل التعديل', true); }
}
async function delIr(c) {
  const used = cmds.some(x => (x.steps || []).some(s => s.type === 'ir' && String(s.val) === String(c.id)));
  if (!confirm('حذف الإشارة "' + c.name + '"؟' + (used ? '\nمستخدمة في أوامر صوتية!' : ''))) return;
  try { const r = await post('/api/ir/delete', {id: c.id}); if (!r.ok) throw 0; toast('تم الحذف'); loadIrList(); }
  catch (e) { toast('فشل الحذف', true); }
}

/* ---------- maintenance: info ---------- */
function fmtUp(s) {
  const d = Math.floor(s / 86400), h = Math.floor(s % 86400 / 3600), m = Math.floor(s % 3600 / 60);
  return (d ? d + 'd ' : '') + h + 'h ' + m + 'm';
}
async function loadInfo() {
  try {
    const i = await (await fetch('/api/info', {cache:'no-store'})).json();
    $('#ip').textContent = i.ip; $('#dot').className = 'dot on';
    const box = $('#info'); box.textContent = '';
    [['IP', i.ip], ['Wi-Fi', i.rssi + ' dBm'], ['RAM', Math.round(i.heap / 1024) + ' KB'],
     ['PSRAM', Math.round(i.psram / 1024 / 1024 * 10) / 10 + ' MB'], ['Uptime', fmtUp(i.up)], ['Wake word', i.wake]]
    .forEach(([a, b]) => { const c = el('div', 'chip'); c.appendChild(el('small', '', a)); c.appendChild(el('b', '', b)); box.appendChild(c); });
  } catch (e) { $('#dot').className = 'dot'; }
}

/* ---------- maintenance: serial monitor ---------- */
let since = 0, carry = '', logTimer = null, infoTimer = null, paused = false, busy = false;
const logLines = [];
function cls(t) {
  if (/error|fail|not found|unable|not recognized/i.test(t)) return 'err';
  if (/wake word detected/i.test(t)) return 'wake';
  if (/executed|sent wol|transcribed|connected|loaded|saved|manual/i.test(t)) return 'ok';
  if (/recording|processing/i.test(t)) return 'proc';
  return '';
}
function addLine(text, c) {
  const term = $('#term');
  const time = new Date().toTimeString().slice(0, 8);
  const d = el('div', c || cls(text));
  d.appendChild(el('span', 't', time)); d.appendChild(document.createTextNode(text));
  term.appendChild(d);
  logLines.push(time + ' ' + text);
  while (term.childNodes.length > 500) term.removeChild(term.firstChild);
  if (logLines.length > 2000) logLines.splice(0, 500);
  if ($('#autoscroll').checked) term.scrollTop = term.scrollHeight;
}
function pushText(txt) {
  const parts = (carry + txt).split('\n');
  carry = parts.pop();
  parts.forEach(p => { p = p.replace(/\r/g, ''); if (p.trim()) addLine(p); });
}
async function pollLog() {
  if (paused || busy) return;
  busy = true;
  try {
    const r = await fetch('/api/log?since=' + since, {cache:'no-store'});
    const next = parseInt(r.headers.get('X-Next') || '0', 10);
    const txt = await r.text();
    if (next < since) { carry = ''; addLine('— الجهاز اتعمله إعادة تشغيل —', 'sys'); }
    since = next;
    if (txt) pushText(txt);
    $('#dot').className = 'dot on';
  } catch (e) { $('#dot').className = 'dot'; }
  busy = false;
}
function startLog() {
  stopLog(); pollLog();
  logTimer = setInterval(pollLog, 1000);
  infoTimer = setInterval(loadInfo, 10000);
}
function stopLog() { clearInterval(logTimer); clearInterval(infoTimer); }
function togglePause() {
  paused = !paused;
  $('#pauseBtn').textContent = paused ? '▶ استئناف' : '⏸ إيقاف';
}
function clearTerm() { $('#term').textContent = ''; logLines.length = 0; post('/api/log/clear').catch(() => {}); }
async function copyLog() {
  try { await navigator.clipboard.writeText(logLines.join('\n')); toast('تم النسخ'); }
  catch (e) {
    const ta = el('textarea'); ta.value = logLines.join('\n'); document.body.appendChild(ta);
    ta.select(); document.execCommand('copy'); ta.remove(); toast('تم النسخ');
  }
}
function downloadLog() {
  const a = el('a'); a.href = URL.createObjectURL(new Blob([logLines.join('\n')], {type:'text/plain'}));
  a.download = 'serial-log.txt'; a.click(); URL.revokeObjectURL(a.href);
}
async function sendSay() {
  const t = $('#say').value.trim(); if (!t) return;
  try { await post('/api/say', {text: t}); $('#say').value = ''; addLine('> ' + t, 'sys'); setTimeout(pollLog, 400); }
  catch (e) { toast('فشل الإرسال', true); }
}
async function reboot() {
  if (!confirm('إعادة تشغيل الجهاز؟')) return;
  try { await post('/api/reboot'); toast('جاري إعادة التشغيل...'); } catch (e) {}
}

/* ---------- نسخة احتياطية: تصدير / استيراد ---------- */
async function exportCmds() {
  try {
    const r = await (await fetch('/api/commands', {cache: 'no-store'})).json();
    let il = []; try { const x = await (await fetch('/api/ir/list', {cache: 'no-store'})).json(); il = Array.isArray(x) ? x : []; } catch (e) {}
    const ir = [];
    for (const i of il) {
      const t = await fetch('/api/ir/raw?id=' + i.id, {cache: 'no-store'});
      if (!t.ok) { toast('فشل قراءة الإشارة: ' + i.name, true); return; }
      ir.push({id: i.id, name: i.name, khz: i.khz, rep: i.rep, pulses: (await t.text()).trim().split(',').map(Number)});
    }
    const data = {app: 'voice-hub', version: 2, exported: new Date().toISOString(), commands: Array.isArray(r) ? r : [], ir};
    const a = el('a'); a.href = URL.createObjectURL(new Blob([JSON.stringify(data, null, 2)], {type: 'application/json'}));
    a.download = 'voice-hub-backup-' + new Date().toISOString().slice(0, 10) + '.json';
    document.body.appendChild(a); a.click(); a.remove(); setTimeout(() => URL.revokeObjectURL(a.href), 1000);
    toast('تم تصدير ' + data.commands.length + ' أمر و ' + ir.length + ' إشارة IR');
  } catch (e) { toast('فشل التصدير', true); }
}
async function importCmds(inp) {
  const f = inp.files[0]; inp.value = ''; if (!f) return;
  let list, irs;
  try {
    const d = JSON.parse(await f.text());
    list = Array.isArray(d) ? d : d.commands;
    if (!Array.isArray(list)) throw 0;
    irs = Array.isArray(d.ir) ? d.ir.filter(x => x && x.name && Array.isArray(x.pulses) && x.pulses.length >= 5) : [];
  } catch (e) { toast('الملف مش صالح', true); return; }
  list = list.filter(c => c && c.name && Array.isArray(c.phrases) && Array.isArray(c.steps));
  if (!list.length && !irs.length) { toast('مفيش بيانات صالحة في الملف', true); return; }
  const replace = confirm('الملف فيه ' + list.length + ' أمر و ' + irs.length + ' إشارة IR.\n\nOK = استبدال كل الموجود حالياً\nCancel = إضافتها على الموجود');
  if (!replace && !confirm('إضافة ' + list.length + ' أمر و ' + irs.length + ' إشارة على الموجود؟')) return;
  try {
    if (replace) {
      await post('/api/clear');
      if (irs.length) {
        const cur = await (await fetch('/api/ir/list', {cache: 'no-store'})).json();
        for (const c of cur) await post('/api/ir/delete', {id: c.id});
      }
    }
    // الإشارات الأول، وبنسجل الرقم الجديد لكل إشارة عشان نعدّل الأوامر عليه
    const map = {}; let irOk = 0;
    for (const i of irs) {
      const q = new URLSearchParams({name: i.name, khz: i.khz || 38, rep: i.rep || 1}).toString();
      const r = await fetch('/api/ir/import?' + q, {method: 'POST', headers: {'Content-Type': 'text/plain'}, body: i.pulses.join(',')});
      if (r.ok) { map[String(i.id)] = (await r.text()).trim(); irOk++; }
    }
    let ok = 0;
    for (const c of list) {
      const steps = c.steps.map(s => (s.type === 'ir' && irs.length) ? {type: 'ir', val: map[String(s.val)] || String(s.val)} : s);
      const r = await fetch('/api/save', {method: 'POST', headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({id: 0, name: c.name, cat: c.cat || 'عام', phrases: c.phrases, steps})});
      if (r.ok) ok++; else break;
    }
    const bad = ok !== list.length || irOk !== irs.length;
    toast('تم استيراد ' + ok + '/' + list.length + ' أمر و ' + irOk + '/' + irs.length + ' إشارة', bad);
    resetForm(); loadIrList().then(loadCommands);
  } catch (e) { toast('فشل الاستيراد', true); }
}

window.onload = () => { fillPins(); loadIrList().then(() => { resetForm(); loadCommands(); }); loadInfo(); let ho = false; try { ho = localStorage.getItem('heardOpen') === '1'; } catch (e) {} toggleHeard(ho); };
</script>
</body>
</html>
)rawliteral";

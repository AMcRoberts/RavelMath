#!/usr/bin/env python3
from __future__ import annotations
import argparse, re
from pathlib import Path

DECL = re.compile(r'^(theorem|lemma)\s+([A-Za-z0-9_\.]+)\b(.*)$')
COMPARTMENT_RULES = [
    ('matrix', ['matrix','det','minor','charpoly','block_identity','cofactor']),
    ('spectral', ['perron','eigen','spectral','gap','root','dominance']),
    ('affine-catalogue', ['affine','catalogue','shell','terminal','neighbor','round','contact']),
    ('word-language', ['swap','word','parikh','occurrence','language']),
    ('graph', ['graph','edge','vertex','scc','contact']),
    ('window', ['window','strip','margin','valid','width','height']),
    ('number-theory', ['geom','pow','polynomial','cubic','integer','ratio']),
]

def classify(name: str, sig: str, path: Path) -> tuple[str,str,list[str]]:
    text=(name+' '+sig+' '+str(path)).lower()
    comps=[]
    tags=[]
    for comp, keys in COMPARTMENT_RULES:
        hit=[k for k in keys if k in text]
        if hit:
            comps.append(comp); tags.extend(hit)
    compartment='+'.join(comps[:3]) if comps else 'general'
    p=str(path).lower()
    if 'draft' in p or 'attempt' in p:
        status='draft'
    elif 'nbonacci_universal_n.kernel_checked.lean' in p:
        status='kernel-checked-export'
    else:
        status='lean-declaration'
    return compartment,status,sorted(set(tags))

def collect_signature(lines, i):
    parts=[lines[i].strip()]
    depth=0
    j=i+1
    while j < len(lines) and j <= i+12:
        s=lines[j].strip()
        if not s or s.startswith('--'): break
        if re.match(r'^(theorem|lemma|def|inductive|structure|namespace|end)\b', s): break
        parts.append(s)
        if ':=' in s or 'where' == s or s.endswith(':= by') or s.endswith(':=') or s.endswith(':= by'):
            break
        j+=1
    return ' '.join(parts)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--root', default='.')
    ap.add_argument('--out', default='config/theorem_capabilities.tsv')
    args=ap.parse_args()
    root=Path(args.root).resolve()
    records=[]
    for path in sorted((root/'lean').rglob('*.lean')):
        lines=path.read_text(encoding='utf-8', errors='replace').splitlines()
        namespace=[]
        for i,line in enumerate(lines):
            s=line.strip()
            if s.startswith('namespace '): namespace.append(s.split(None,1)[1].strip())
            elif s.startswith('end ') and namespace: namespace.pop()
            m=DECL.match(s)
            if not m: continue
            kind,name,_=m.groups()
            sig=collect_signature(lines,i)
            comp,status,tags=classify(name,sig,path.relative_to(root))
            fq='.'.join(namespace+[name]) if namespace else name
            records.append((fq,kind,str(path.relative_to(root)),i+1,comp,status,','.join(tags),sig.replace('\t',' ')))
    out=root/args.out
    out.parent.mkdir(parents=True,exist_ok=True)
    with out.open('w',encoding='utf-8') as f:
        f.write('name\tkind\tfile\tline\tcompartment\tstatus\ttags\tsignature\n')
        for r in records:
            f.write('\t'.join(map(str,r))+'\n')
    print(f'wrote {len(records)} theorem capabilities to {out}')
if __name__=='__main__': main()

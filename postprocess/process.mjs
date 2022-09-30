#!/usr/bin/node

import fs from 'node:fs';

const dic = JSON.parse(fs.readFileSync(process.argv[2]));

for (const k in dic) {
    const t = dic[k].n * dic[k].speed / 8 / 60;
    dic[k] = {
        r: dic[k].rate,
        n: dic[k].n,
        t: t >= 120 ? Math.round(t / 60) + ' hours' : Math.round(t) + ' minutes',
    };
}

const g = (hsh) => dic[hsh] || { r: 'TODO', n: 100000000, t: '???' };

const breaking = (s) => s.replace(/(?=[+\u00b1])/, '<br>');

const print_result = (c, ij) => `
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>${g(`FL@[1,1]-PSEQ-${c}-SFAR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-${c}-SFAR ${g(`FL@[1,1]-PSEQ-${c}-SFAR`).n}</pre>
              Approx. takes ${g(`FL@[1,1]-PSEQ-${c}-SFAR`).t} to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>${g(`FL@[1,1]-PSEQ-D256-${c}-SFAR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-${c}-SFAR ${g(`FL@[1,1]-PSEQ-D256-${c}-SFAR`).n}</pre>
              Approx. takes ${g(`FL@[1,1]-PSEQ-D256-${c}-SFAR`).t} to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>${g(`FL@[${ij}]-PSEQ-${c}-SNR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[${ij}]-PSEQ-${c}-SNR ${g(`FL@[${ij}]-PSEQ-${c}-SNR`).n}</pre>
              Approx. takes ${g(`FL@[${ij}]-PSEQ-${c}-SNR`).t} to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>${g(`FL@[${ij}]-PSEQ-D256-${c}-SNR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[${ij}]-PSEQ-D256-${c}-SNR ${g(`FL@[${ij}]-PSEQ-D256-${c}-SNR`).n}</pre>
              Approx. takes ${g(`FL@[${ij}]-PSEQ-D256-${c}-SNR`).t} to run on an 8-core machine.</details></td>`;

console.log(`
# MineSweeperProb
A deterministic Minesweeper solver

## Build

${'```'}bash
sudo pacman -S cmake ninja boost
cmake -S MineSweeperSolver -B cmake-bulid-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-mnative' -G Ninja
cmake --build cmake-build-release
${'```'}

## Winning Rate

<table>
    <thead>
        <tr>
            <th>Board Size</th>
            <th>Rule</th>
            <th>Strategy</th>
            <th>Rate</th>
            <th>Cmd</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=4>8x8<br>10 mines</td>${print_result('8-8-T10', '3,3')}
        </tr>
        <tr>
            <td rowspan=4>9x9<br>10 mines</td>${print_result('9-9-T10', '3,3')}
        </tr>
        <tr>
            <td rowspan=4>16x16<br>40 mines</td>${print_result('16-16-T40', '3,3')}
        </tr>
        <tr>
            <td rowspan=4>30x16<br>99 mines</td>${print_result('30-16-T99', '4,4')}
        </tr>
    </tbody>
</table>
`);

const print_array = (c, w, h, best) => {
    const m = c.match(/^(?<w>[0-9]+)-(?<h>[0-9]+)-*/);
    const ww = +m.groups.w, hh = +m.groups.h;
    console.log(`
<table>
    <thead>
        <tr>
            <th></th>`);
    for (let i = 1; i <= w; i++)
        console.log(`            <td>${i}</td>`);
    console.log(`        </tr>
    </thead>
    <tbody>`);
    for (let j = 1; j <= h; j++) {
        console.log(`        <tr>
            <th>${j}</th>`);
        for (let i = 1; i <= w; i++) {
            const good = (j === best[0] || j + best[0] == hh + 1) &&
                (i === best[1] || i + best[1] == ww + 1);
            console.log(`            <td>${good ? '<strong>' : ''}${breaking(g(`FL@[${j},${i}]-PSEQ-${c}`).r)}${good ? '</strong>' : ''}</td>`);
        }
        console.log(`        </tr>`);
    }
    console.log(`    </tbody>
</table>
`);
};

console.log(`

## Winning Rate by First Move

`);

console.log(`### 8x8, 10 mines`);
console.log(`SFAR`);
print_array('8-8-T10-SFAR', 8, 8, [1, 1]);
console.log(`SNR`);
print_array('8-8-T10-SNR', 8, 8, [3, 3]);

console.log(`### 9x9, 10 mines`);
console.log(`SFAR`);
print_array('9-9-T10-SFAR', 9, 9, [1, 1]);
console.log(`SNR`);
print_array('9-9-T10-SNR', 9, 9, [3, 3]);

console.log(`### 16x16, 40 mines`);
console.log(`SFAR`);
print_array('16-16-T40-SFAR', 8, 8, [1, 1]);
console.log(`SNR`);
print_array('16-16-T40-SNR', 8, 8, [3, 3]);

console.log(`### 30x16, 99 mines`);
console.log(`SFAR`);
print_array('30-16-T99-SFAR', 15, 8, [1, 1]);
console.log(`SNR`);
print_array('30-16-T99-SNR', 15, 8, [4, 4]);

#!/usr/bin/node

import fs from 'node:fs';

const dic = JSON.parse(fs.readFileSync(process.argv[2]));

for (const k in dic) {
    const t = 1e8 * dic[k].speed / 8 / 60;
    dic[k] = {
        r: dic[k].rate,
        t: t >= 120 ? Math.round(t / 60) + ' hours' : Math.round(t) + ' minutes',
    };
}

const g = (hsh) => dic[hsh] || { r: 'TODO', t: '???' };

const f = (c) => `
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>${g(`FL@[1,1]-PSEQ-${c}-SFAR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-${c}-SFAR 100000000</pre>
              Approx. takes ${g(`FL@[1,1]-PSEQ-${c}-SFAR`).t} to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>${g(`FL@[1,1]-PSEQ-D256-${c}-SFAR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-${c}-SFAR 100000000</pre>
              Approx. takes ${g(`FL@[1,1]-PSEQ-D256-${c}-SFAR`).t} to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>${g(`FL@[3,3]-PSEQ-${c}-SNR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-${c}-SNR 100000000</pre>
              Approx. takes ${g(`FL@[3,3]-PSEQ-${c}-SNR`).t} to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>${g(`FL@[3,3]-PSEQ-D256-${c}-SNR`).r}</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-${c}-SNR 100000000</pre>
              Approx. takes ${g(`FL@[3,3]-PSEQ-D256-${c}-SNR`).t} to run on an 8-core machine.</details></td>`;

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
            <td rowspan=4>8x8<br>10 mines</td>${f('8-8-T10')}
        </tr>
        <tr>
            <td rowspan=4>9x9<br>10 mines</td>${f('9-9-T10')}
        </tr>
        <tr>
            <td rowspan=4>16x16<br>40 mines</td>${f('16-16-T40')}
        </tr>
        <tr>
            <td rowspan=4>30x16<br>99 mines</td>${f('30-16-T99')}
        </tr>
    </tbody>
</table>
`);

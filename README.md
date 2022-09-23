
# MineSweeperProb
A deterministic Minesweeper solver

## Build

```bash
sudo pacman -S cmake ninja boost
cmake -S MineSweeperSolver -B cmake-bulid-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-mnative' -G Ninja
cmake --build cmake-build-release
```

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
            <td rowspan=4>8x8<br>10 mines</td>
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>TODO</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>TODO</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-8-8-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>90.1456±0.0026%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-8-8-T10-SNR 100000000</pre>
              Approx. takes 10 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>90.331±0.014%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-8-8-T10-SNR 100000000</pre>
              Approx. takes 182 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=4>9x9<br>10 mines</td>
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>TODO</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>TODO</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-9-9-T10-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>97.1453±0.0015%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-9-9-T10-SNR 100000000</pre>
              Approx. takes 8 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>97.1669±0.0033%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-9-9-T10-SNR 100000000</pre>
              Approx. takes 19 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=4>16x16<br>40 mines</td>
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>TODO</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>TODO</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-16-16-T40-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>88.9267±0.0031%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-16-16-T40-SNR 100000000</pre>
              Approx. takes 40 minutes to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>89.080±0.013%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-16-16-T40-SNR 100000000</pre>
              Approx. takes 92 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=4>30x16<br>99 mines</td>
            <td rowspan=2>SFAR</td>
            <td>PSEQ</td>
            <td>TODO</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>TODO</td>
            <td><details><pre>./MineSweeperSolver FL@[1,1]-PSEQ-D256-30-16-T99-SFAR 100000000</pre>
              Approx. takes ??? to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td rowspan=2>SNR</td>
            <td>PSEQ</td>
            <td>51.8195±0.0098%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-30-16-T99-SNR 100000000</pre>
              Approx. takes 7 hours to run on an 8-core machine.</details></td>
        </tr>
        <tr>
            <td>PSEQ-D256</td>
            <td>52.491±0.047%</td>
            <td><details><pre>./MineSweeperSolver FL@[3,3]-PSEQ-D256-30-16-T99-SNR 100000000</pre>
              Approx. takes 802 hours to run on an 8-core machine.</details></td>
        </tr>
    </tbody>
</table>


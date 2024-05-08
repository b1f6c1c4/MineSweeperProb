import React from 'react';

export default function HeatBar(props) {
    const {
        disabled,
        isSafe,
        isDrain,
        isDangerous,
        hasSafe,
        probability,
        minProb,
        maxProb,
    } = props;

    let fmt = ''+Math.round(probability * 1000) / 10;
    if (!/\./.test(fmt))
        fmt += '.';
    if (fmt.length < 4)
        fmt += '0'.repeat(4 - fmt.length);

    let btm = minProb;
    let lng = maxProb - minProb;
    let col;
    let opa;
    if (lng < 0) {
        btm = 0;
        lng = 1;
        opa = '15%';
    } else if (lng === 0) {
        const probability = (maxProb + minProb) / 2
        lng = 0.05;
        btm = probability - lng / 2;
        if (!isDrain) {
            const r = Math.round(0xf8 * probability + 0x09 * (1 - probability));
            const g = Math.round(0x29 * probability + 0xb7 * (1 - probability));
            const b = Math.round(0x29 * probability + 0x22 * (1 - probability));
            col = `rgb(${r}, ${g}, ${b})`;
        } else {
            const r = Math.round(0xf8 * probability + 0x7b * (1 - probability));
            const g = Math.round(0x29 * probability + 0x68 * (1 - probability));
            const b = Math.round(0x29 * probability + 0xee * (1 - probability));
            col = `rgb(${r}, ${g}, ${b})`;
        }
    }

    return (
        <div className="heat-bar">
            <div className="ends dangerous">
                <span className="color"></span>
                {isDangerous ? (
                    <span className="indicator here">100% Death</span>
                ) : (
                    <span className="indicator">
                        {isDrain ? 'Lose' : 'Mine'}
                    </span>
                )}
            </div>
            <div className="mid">
                <span className="color">
                    <span className="color" style={{
                            bottom: `${btm * 100}%`,
                            height: `${lng * 100}%`,
                            backgroundColor: col,
                            backgroundImage: col ? 'initial' : undefined,
                            opacity: opa,
                        }}></span>
                </span>
                {probability !== null && (
                    <span className="indicator here" style={{
                            bottom: `${probability * 100}%`,
                        }}>
                        {fmt}%
                    </span>
                )}
            </div>
            <div className={'ends safe' + (hasSafe ? '' : ' unavailable')}>
                <span className="color"></span>
                {isSafe ? (
                    <span className="indicator here">100% Safe</span>
                ) : (
                    <span className="indicator">
                        {isDrain ? 'Win' : 'Empty'}
                    </span>
                )}
            </div>
        </div>
    );
}

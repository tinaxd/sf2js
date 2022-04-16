let sf2 = null;

let load_sf2;
let note_on;
let note_off;
let render_float;

let audioPort;

function loadSf2() {
    load_sf2 = Module.cwrap('load_sf2', 'number', ['string']);
    sf2 = load_sf2("FluidR3_GM.sf2");
}

function noteOn(key, vel) {
    if (note_on === undefined) {
        note_on = Module.cwrap('note_on', null, ['number', 'number', 'number', 'number']);
    }
    note_on(sf2, 1, key, vel);
    console.log('note on');
}

function noteOff(key) {
    if (note_off === undefined) {
        note_off = Module.cwrap('note_off', null, ['number', 'number', 'number']);
    }
    note_off(sf2, 1, key);
}

function renderFloat(samples) {
    const floatArray = new Float32Array(samples);
    if (render_float === undefined) {
        render_float = Module.cwrap('render_float', null, ['number', 'number', 'number']);
    }
    render_float(sf2, floatArray, samples);
    return floatArray;
}

function onReady() {
    loadSf2();
    postMessage({
        type: 'finish-loading'
    });
}
importScripts("tfs.js");

onmessage = (ev) => {
    const data = ev.data;

    switch (data.type) {
        case 'pass-port':
            audioPort = data.port;
            console.log('received audioPort');

            audioPort.onmessage = (ev) => {
                switch (ev.data.type) {
                    case 'render-float':
                        const array = renderFloat(ev.data.samples);
                        audioPort.postMessage({
                            type: 'render-float-result',
                            buffer: array.buffer
                        }, [array.buffer]);
                        break;
                }
            };

            return;
    }

    if (sf2 === null) return;
    switch (data.type) {
        case 'note-on':
            noteOn(data.key, data.vel);
            break;
        case 'note-off':
            noteOff(data.key);
            break;
    }
}

import Module from './myeffect.wasmmodule.js';
import { RENDER_QUANTUM_FRAMES, MAX_CHANNEL_COUNT, HeapAudioBuffer }
    from './wasm-audio-helper.js';


/**
 * A simple demonstration of WASM-powered AudioWorkletProcessor.
 *
 * @class WASMWorkletProcessor
 * @extends AudioWorkletProcessor
 */
class WorkletProcessor extends AudioWorkletProcessor {

  static get parameterDescriptors() {
    return [{ name: 'dist', defaultValue: 50.0, minValue: 0.0, maxValue: 100.0 }, { name: 'freq',  defaultValue: 4000.0, minValue: 0.0, maxValue: 20000.0 }, 
	    { name: 'bypass', defaultValue: true }];
  }

  /**
   * @constructor
   */
  constructor() {
    super();

    // Allocate the buffer for the heap access. Start with stereo, but it can
    // be expanded up to 32 channels.
    this._heapInputBuffer = new HeapAudioBuffer(Module, RENDER_QUANTUM_FRAMES,
                                                2, MAX_CHANNEL_COUNT);
    this._heapOutputBuffer = new HeapAudioBuffer(Module, RENDER_QUANTUM_FRAMES,
                                                 2, MAX_CHANNEL_COUNT);

    this._kernel = new Module.Distortion();
  }

  /**
   * System-invoked process callback function.
   * @param  {Array} inputs Incoming audio stream.
   * @param  {Array} outputs Outgoing audio stream.
   * @param  {Object} parameters AudioParam data.
   * @return {Boolean} Active source flag.
   */
  process(inputs, outputs, parameters) {
    // Use the 1st input and output only to make the example simpler. |input|
    // and |output| here have the similar structure with the AudioBuffer
    // interface. (i.e. An array of Float32Array)
    let input = inputs[0];
    let output = outputs[0];

    // For this given render quantum, the channel count of the node is fixed
    // and identical for the input and the output.
    let channelCount = input.length;

    // Prepare HeapAudioBuffer for the channel count change in the current
    // render quantum.
    this._heapInputBuffer.adaptChannel(channelCount);
    this._heapOutputBuffer.adaptChannel(channelCount);

    // Copy-in, process and copy-out.
    for (let channel = 0; channel < channelCount; ++channel) {
      this._heapInputBuffer.getChannelData(channel).set(input[channel]);
    }

    const dist = parameters.dist;
    this._kernel.distortion = dist[0];

    const freq = parameters.freq;
    this._kernel.cutoff = freq[0];

    const bypass = parameters.bypass;
    this._kernel.bypass = bypass[0];

    this._kernel.process(this._heapInputBuffer.getHeapAddress(),
                         this._heapOutputBuffer.getHeapAddress(),
                         channelCount);
    for (let channel = 0; channel < channelCount; ++channel) {
      output[channel].set(this._heapOutputBuffer.getChannelData(channel));
    }

    return true;
  }
}


registerProcessor('worklet-processor', WorkletProcessor);

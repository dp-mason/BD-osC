#include "plugin.hpp"
#include <patch.hpp>

// Function to convert a vector of floats to a CSV string
void saveKeyframesToCSV(const std::list<std::vector<float>>& kyfrms, const std::string parentFolderPath) {
	
	std::ostringstream csvStream;

	// Make a row out of each keyframe
	for (const auto& kframe : kyfrms) {
		for (size_t track = 0; track < kframe.size(); track++) {
			csvStream << std::to_string( kframe[track] );
			if(track < kframe.size() - 1){
				csvStream << ",";
			}
		}

		csvStream << "\n";
	}

	// Create or open the CSV file and write the CSV data
	std::ofstream csvFile( parentFolderPath + "keyframes.csv" );
	if (csvFile.is_open()) {
		csvFile << csvStream.str();
		csvFile.close();
	}

	return;
}

// Function to convert a vector of floats to a CSV string
void saveWfKeyframesToCSV(const std::list<std::vector<std::vector<float>>>& wfKyfrms, const std::string parentFolderPath) {
	
	if(wfKyfrms.size() == 0) {
		return;
	}

	std::vector<std::ostringstream> csvStreams;
	csvStreams.resize(5);

	for (const auto& kframe : wfKyfrms) {
		for (size_t wave = 0; wave < 5; wave++) {
			for (size_t sample = 0; sample < kframe[0].size(); sample++) {
				csvStreams[wave] << std::fixed << std::setprecision(3) << kframe[wave][sample];
				if(sample < kframe[0].size() - 1){
					csvStreams[wave] << ",";
				}
			}

			csvStreams[wave] << "\n";
		}
	}

	// Create or open the CSV file and write the CSV data
	for (int wave = 0; wave < 5; wave++) {
		std::ofstream csvFile( parentFolderPath + "wave_" + std::to_string(wave + 1) + "_keyframes.csv" );
		if (csvFile.is_open()) {
			csvFile << csvStreams[wave].str();
			csvFile.close();
		}
	}

	return;
}

void saveFrameRateToTxt(int64_t frame_rate, const std::string parentFolderPath){
	std::ofstream txtFile( parentFolderPath + "framerate.txt" );

	if (txtFile.is_open()) {
		txtFile << std::to_string(frame_rate);
		txtFile.close();
	}
}

struct BD_osC : Module {

	// TODO: connect the keyframeRate up with the appropriate param input
	int64_t keyframeRate = 30; // stored in this data type so that there is less casting per process call
	int64_t startFrame = 0; // this value is set when the RECORD input is triggered
	bool recordingActive = false; // determines whether keyframes will be added
	// each row is a keyframe, each column is a track. Makes it easier to append values (and prob better for mem management)
	// this means there will be 16 columns, one for each track
	std::list<std::vector<float>> keyframes;
	
	float prevSaveVoltage  = 0.f;
	float prevStartVoltage = 0.f;
	float prevStopVoltage  = 0.f;

	int64_t samplesInVisualFrame;

	// Determines the resolution of a visualized waveform
	int64_t maxWfResolution = 128;

    // Set the dimensions, number of waveforms
    size_t numWfs = 5;

	std::vector<bool> wfRecordState = std::vector<bool>(numWfs, false); // TODO: wtf is this for again? Unused.

	// Define a matrix representing the current state of all the waveforms
    std::vector<std::vector<float>> currWfKframeState;

	// Define a tensor representing the visual keyframes of the waves over time
	// At the end of each keyframe, the matrix representing the current state of all waveforms will be appended
	// Think of it like every keyframe is a sheet of paper with 5 waveform states, each time a keyframe is added
	// 	a new sheet of paper is placed at the bottom of the stack (the memory isnt moved as if it were a "stack" though)
	// The "list" data type should help with resizing/data corruption issues that I was experiencing
    std::list<std::vector<std::vector<float>>> waveformKeyframes;

	enum ParamId {
		FRAME_RATE_PARAM,
		WAVE_SAMPLE_RATE_PARAM,
		PARAMS_LEN
	};
	// CAUTION: THE ORDER OF THE INPUTS MATTERS FOR LATER LOOPS
	// Add any new inputs to the end
	enum InputId {
		WAVE_I_INPUT,
		WAVE_II_INPUT,
		WAVE_III_INPUT,
		WAVE_IV_INPUT,
		WAVE_V_INPUT,
		VOCT_I_INPUT,
		VOCT_II_INPUT,
		VOCT_III_INPUT,
		VOCT_IV_INPUT,
		VOCT_V_INPUT,
		START_INPUT,
		ABORT_INPUT,
		SAVE_INPUT,
		INPUT_1_INPUT,
		INPUT_2_INPUT,
		INPUT_3_INPUT,
		INPUT_4_INPUT,
		INPUT_5_INPUT,
		INPUT_6_INPUT,
		INPUT_7_INPUT,
		INPUT_8_INPUT,
		INPUT_9_INPUT,
		INPUT_10_INPUT,
		INPUT_11_INPUT,
		INPUT_12_INPUT,
		INPUT_13_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(RECORD_LIGHT, 3),
		LIGHTS_LEN
	};

	BD_osC() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(FRAME_RATE_PARAM, 0.f, 4.f, 0.f, "");
		paramQuantities[FRAME_RATE_PARAM]->snapEnabled = true;
		configParam(WAVE_SAMPLE_RATE_PARAM, 0.f, 3.f, 0.f, "");
		paramQuantities[WAVE_SAMPLE_RATE_PARAM]->snapEnabled = true;
		configInput(START_INPUT, "");
		configInput(ABORT_INPUT, "");
		configInput(SAVE_INPUT, "");
		configInput(WAVE_I_INPUT, "");
		configInput(WAVE_II_INPUT, "");
		configInput(WAVE_III_INPUT, "");
		configInput(WAVE_IV_INPUT, "");
		configInput(WAVE_V_INPUT, "");
		configInput(VOCT_I_INPUT, "");
		configInput(VOCT_II_INPUT, "");
		configInput(VOCT_III_INPUT, "");
		configInput(VOCT_IV_INPUT, "");
		configInput(VOCT_V_INPUT, "");
		configInput(INPUT_1_INPUT, "");
		configInput(INPUT_2_INPUT, "");
		configInput(INPUT_3_INPUT, "");
		configInput(INPUT_4_INPUT, "");
		configInput(INPUT_5_INPUT, "");
		configInput(INPUT_6_INPUT, "");
		configInput(INPUT_7_INPUT, "");
		configInput(INPUT_8_INPUT, "");
		configInput(INPUT_9_INPUT, "");
		configInput(INPUT_10_INPUT, "");
		configInput(INPUT_11_INPUT, "");
		configInput(INPUT_12_INPUT, "");
		configInput(INPUT_13_INPUT, "");
	}

	// resets the average values of the inputs that record keyframes of the average value rather than a straight up sample
	// resets the recorded waveforms to zero
	void resetCurrKfData(){
		for(size_t currWave = 0; currWave < numWfs; currWave++){
			currWfKframeState[currWave] = {std::vector<float>(maxWfResolution, 0.f)};
		}
	}

	void clearStoredKfs(){
		keyframes.clear();
		waveformKeyframes.clear();
	}

	void saveKfsToDisk(std::string parent_folder){
		// TODO: maybe spawn one async thread that saves all these files
		// TODO: pass by ref and then make sure kf data is not erased or manipulated until done instead of pass by value
		// std::async(saveKeyframesToCSV, keyframes, parent_folder + "keyframes.csv");
		saveKeyframesToCSV(keyframes, parent_folder);
		// std::async(saveWfKeyframesToCSV, waveformKeyframes, parent_folder);
		saveWfKeyframesToCSV(waveformKeyframes, parent_folder);
		// Save the frame rate to disk
		saveFrameRateToTxt(keyframeRate, parent_folder);
	}

	float voctToHz(float voctVoltage){
		return (440.f / 2.f) * pow(2.f, (voctVoltage + 0.25));
	}

	int64_t processWf(
		const ProcessArgs& args, 
		std::vector<float>& waveKf, 
		float voltage, 
		float voctCV, 
		int64_t samplesInVisKf, 
		int64_t currFrameInKf
	){
		// TODO: save an index for the number of samples in the wave?

		// use v/oct to capture a window if the signal the length of 1 wavelength
		// determine the number of audio samples in one wavelength of this wave
		// input of less than -99.0 is meant to indicate that the v/oct input is disconnected
		float samplesInWavelength = args.sampleRate / voctToHz(voctCV);
		
		bool wavelen_fits = (samplesInWavelength <= maxWfResolution);
		float timeRatio = wavelen_fits ? 1.000f : (maxWfResolution / samplesInWavelength);

		int64_t sample_index = int64_t( round(fmod(float(args.frame), samplesInWavelength) * timeRatio) ) % maxWfResolution;
					
		// With the longer waveforms certain samples are getting overwitten over and over. The effect of only using the data from the last overwrite
		// is that the waveform looks jagged. handling waveforms that fit and those that do not separately helps
		// TODO: weighing the most recent sample in the same slot more might break down in some situations idk, looks ok with minimal tests
		// TODO: I think it would be cool to optionally do averaging of how the waveform changes over the course of the visual keyframe
		//			rather than only keeping the most recent frame of data 
		if(wavelen_fits){
			waveKf[sample_index] = std::round(voltage * 1000) / 1000; //(waveKf[sample_index] * 0.5) + (voltage * 0.5);
		}
		else{
			// there really isn't much of a reason to use 0.6 and 0.4 here... It makes the waveforms look right though, and its simple...
			waveKf[sample_index] = std::round( (waveKf[sample_index]*0.4 + voltage*0.6) * 1000 ) / 1000.0;
		}

		// return the sample index for debug purposes
		return sample_index;
	}

	void process(const ProcessArgs& args) override {

		// activate recording if the RECORD input is triggered
		if(prevStartVoltage == 0.f && inputs[START_INPUT].getVoltage() > prevStartVoltage){
			recordingActive = true;
			startFrame = args.frame;

			currWfKframeState = std::vector<std::vector<float>>(numWfs);
			for(size_t currWave = 0; currWave < numWfs; currWave++){
				currWfKframeState[currWave] = {std::vector<float>(maxWfResolution, 0.f)};
			}

			
		}
		prevStartVoltage = inputs[START_INPUT].getVoltage();

		// deactivate recording and clear keyframes of the ABORT input is triggered
		if(prevStopVoltage == 0.f && inputs[ABORT_INPUT].getVoltage() > prevStopVoltage){
			clearStoredKfs();
			resetCurrKfData();

			recordingActive = false;
		}
		prevStopVoltage = inputs[ABORT_INPUT].getVoltage();
		
		if(recordingActive) {
			// Turn on recording light. Magic number bad, I know
			lights[RECORD_LIGHT + 0].setBrightness(1.0);
			lights[RECORD_LIGHT + 1].setBrightness(0.0);
			lights[RECORD_LIGHT + 2].setBrightness(0.0);
			
			int64_t currFrameInKf = (args.frame - startFrame) % samplesInVisualFrame;
			// int fractionOfAvg = ( 1.0 / float(samplesInVisualFrame) );
			
			// process the waveform inputs, the IDs run from 0..5 with the VOCT inputs occupying the next 5 spots
			for ( int wf_id = WAVE_I_INPUT; wf_id <= WAVE_V_INPUT; wf_id++ ){
				// Do not process this waveform if input is disconnected
				if (inputs[wf_id].isConnected()){
					// if the v/oct input of this waveform is disconnected, communicate that by sending a tiny value to keyframe func
					float voct_voltage = (inputs[wf_id + 5].isConnected()) ? inputs[wf_id + 5].getVoltage() : -100.0;
					processWf(args, currWfKframeState[wf_id], inputs[wf_id].getVoltage(), voct_voltage, samplesInVisualFrame, currFrameInKf);
					// DEBUG - REMOVE LATER 
					// if(std::abs(currWfKframeState[0][132]) > 0.01 && args.frame % 120 == 0) {
					// 	DEBUG("DURING WAVE: %d\nVALUE OF waveform 0 at index 132 has CHANGED TO: %f\nDURING THE PROCESSING OF SAMPLE: %ld\nTOTAL FRAME: %ld", wf_id, currWfKframeState[0][132], sample, args.frame);
					// }
				}
				else {
					continue;
				}
			}

			// if it is determined that this is the last audio frame of the visual keyframe, save the values to the list of keyframes
			if(currFrameInKf == 0){
				// output the value of the keyframe
				// converts this frame to a value between 0..1 depending on the frame it is in this second 1/24, 2/24, ...
				//outputs[FRAME_START_OUTPUT].setVoltage( (float)((args.frame / samplesInVisualFrame) % keyframeRate) / (float)(keyframeRate) );
				
				std::vector<float> thisKeyframe = {
					inputs[INPUT_1_INPUT].getVoltage(),
					inputs[INPUT_2_INPUT].getVoltage(),
					inputs[INPUT_3_INPUT].getVoltage(),
					inputs[INPUT_4_INPUT].getVoltage(),
					inputs[INPUT_5_INPUT].getVoltage(),
					inputs[INPUT_6_INPUT].getVoltage(),
					inputs[INPUT_7_INPUT].getVoltage(),
					inputs[INPUT_8_INPUT].getVoltage(),
					inputs[INPUT_9_INPUT].getVoltage(),
					inputs[INPUT_10_INPUT].getVoltage(),
					inputs[INPUT_11_INPUT].getVoltage(),
					inputs[INPUT_12_INPUT].getVoltage(),
					inputs[INPUT_13_INPUT].getVoltage(),
				};

				keyframes.push_back(thisKeyframe);

				waveformKeyframes.push_back(currWfKframeState);					

				// prepare for a new keyframe
				resetCurrKfData();

				DEBUG("number of keyframes is %ld", keyframes.size());

			}
		} 
		else {
			// Turn off recording light
			lights[RECORD_LIGHT + 0].setBrightness(0.000);
			lights[RECORD_LIGHT + 1].setBrightness(0.000);
			lights[RECORD_LIGHT + 2].setBrightness(0.000);
			// Allow the frame rate and waveform resolution to be changed with param knobs when recording is not active
			switch(int(params[FRAME_RATE_PARAM].getValue())){
				case 0:
					keyframeRate = 24;
					break;
				case 1:
					keyframeRate = 25;
					break;
				case 2:
					keyframeRate = 30;
					break;
				case 3:
					keyframeRate = 50;
					break;
				case 4:
					keyframeRate = 60;
					break;
				default:
					DEBUG("UNHANDLED PARAM VALUE FOR FRAME RATE: %f", params[FRAME_RATE_PARAM].getValue());
					keyframeRate = 24;
			}
			switch(int(params[WAVE_SAMPLE_RATE_PARAM].getValue())){
				case 0:
					maxWfResolution = 32;
					break;
				case 1:
					maxWfResolution = 64;
					break;
				case 2:
					maxWfResolution = 128;
					break;
				case 3:
					maxWfResolution = 256;
					break;
				default:
					DEBUG("UNHANDLED PARAM VALUE FOR SAMPLE RATE: %f", params[WAVE_SAMPLE_RATE_PARAM].getValue());
					keyframeRate = 32;
			}

			// Lock in the visual framerate, the number of audio samples that are in each visual frame 
			// TODO: this way for tracking the currframe in kf will eventually be out of sync over time, prob not relevant, but worth noting
			//		-- David (06-28-2024): Will it really though? Why?
			// TODO: maybe this should be calles "samplesInKeyframe" for understandability
			samplesInVisualFrame = (int64_t)args.sampleRate / keyframeRate;
		}

		// asynchronously save the keyframes to disk as a CSV file upon trigger
		// TODO: for some reason this segfaults or something when triggered with a gate, not entirely sure why
		if(prevSaveVoltage == 0.f && inputs[SAVE_INPUT].getVoltage() > prevSaveVoltage){
			DEBUG("Saving Keyframes... ");
			auto parent_folder = APP->patch->path.substr(0, APP->patch->path.rfind("/") + 1);
			DEBUG("Saving Data to \"%s\"", parent_folder.c_str());

			DEBUG("rows in keyframes %ld", keyframes.size());
			if( keyframes.size() > 0 ){
				DEBUG("cols in keyframes %ld", (keyframes.front()).size());
			}
			DEBUG("framerate: %ld", keyframeRate);
			
			saveKfsToDisk(parent_folder);
			
			clearStoredKfs();
			resetCurrKfData();
			
			recordingActive = false;
		}

		prevSaveVoltage = inputs[SAVE_INPUT].getVoltage();
	}
};


struct BD_osCWidget : ModuleWidget {
	BGPanel *pBackPanel;
	
	BD_osCWidget(BD_osC* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/BD-osC.svg")));

		// Adapted from https://github.com/netboy3/MSM-vcvrack-plugin/
		pBackPanel = new BGPanel();
		pBackPanel->box.size = box.size;
		pBackPanel->imagePath = asset::plugin(pluginInstance, "res/BD-osC.jpg");
		pBackPanel->visible = false;
		addChild(pBackPanel);
		pBackPanel->visible = true;
		
		RoundBigBlackKnob* fr_knob = createParamCentered<RoundBigBlackKnob>(mm2px(Vec(150.25, 18.39)), module, BD_osC::FRAME_RATE_PARAM);
		fr_knob->minAngle = -1.0 * (M_PI * 0.52);
		fr_knob->maxAngle = M_PI * 0.52;
		addParam(fr_knob);
		
		RoundBlackKnob* wsr_knob = createParamCentered<RoundBlackKnob>(mm2px(Vec(160.08, 49.694)), module, BD_osC::WAVE_SAMPLE_RATE_PARAM);
		wsr_knob->minAngle = 0.0 - (M_PI * 0.57);
		wsr_knob->maxAngle = M_PI * 0.32;
		addParam(wsr_knob);
		
		addChild(createLightCentered<LargeLight<RedGreenBlueLight>>(mm2px(Vec(118.1, 24.1)), module, BD_osC::RECORD_LIGHT));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28.534, 13.424)), module, BD_osC::START_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(59.936, 13.354)), module, BD_osC::ABORT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(90.878, 19.109)), module, BD_osC::SAVE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.611, 45.975)), module, BD_osC::INPUT_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(106.828, 46.561)), module, BD_osC::WAVE_I_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.871, 47.059)), module, BD_osC::INPUT_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.502, 49.044)), module, BD_osC::INPUT_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(134.556, 49.003)), module, BD_osC::WAVE_II_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(85.815, 51.288)), module, BD_osC::INPUT_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(115.716, 56.7)), module, BD_osC::VOCT_I_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(143.609, 59.145)), module, BD_osC::VOCT_II_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(53.596, 68.292)), module, BD_osC::INPUT_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.335, 68.777)), module, BD_osC::INPUT_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(95.721, 71.242)), module, BD_osC::WAVE_III_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(76.645, 71.479)), module, BD_osC::INPUT_7_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(123.479, 74.974)), module, BD_osC::WAVE_IV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(151.449, 75.38)), module, BD_osC::WAVE_V_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(104.727, 81.414)), module, BD_osC::VOCT_III_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(132.461, 85.203)), module, BD_osC::VOCT_IV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(160.423, 85.545)), module, BD_osC::VOCT_V_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.608, 90.196)), module, BD_osC::INPUT_9_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.837, 91.32)), module, BD_osC::INPUT_10_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.532, 93.388)), module, BD_osC::INPUT_8_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(85.809, 95.59)), module, BD_osC::INPUT_11_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(108.676, 100.108)), module, BD_osC::INPUT_12_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(131.844, 102.18)), module, BD_osC::INPUT_13_INPUT));
	}
};


Model* modelBD_osC = createModel<BD_osC, BD_osCWidget>("BD_osC");
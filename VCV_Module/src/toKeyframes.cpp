#include "plugin.hpp"

#include <future>
#include <fstream>

// Function to convert a vector of floats to a CSV string
void saveKeyframesToCSV(const std::vector<std::vector<float>>& kyfrms) {
	std::ostringstream csvStream;

	for (size_t frame = 0; frame < kyfrms.size(); frame++) {
		for (size_t track = 0; track < 16; track++) {
			csvStream << std::to_string( kyfrms[frame][track] );
			if(frame != kyfrms.size()){
				csvStream << ",";
			}
		}

		csvStream << "\n";
	}

	std::string csvString = csvStream.str();

	// Specify the CSV file path
	std::string filePath = "${VCV_DEV}/VCV-Keyframes/keyframes.csv";

	// Create or open the CSV file and write the CSV data
	std::ofstream csvFile(filePath);
	if (csvFile.is_open()) {
		csvFile << csvString;
		csvFile.close();
	}
}

struct ToKeyframes : Module {

	int64_t keyframeRate = 24; // stored in this data type so that there is less casting per process call
	int64_t startFrame = 0; // this value is set when the RECORD input is triggered
	bool recordingActive = false; // determines whether keyframes will be added
	// each row is a keyframe, each column is a track. Makes it easier to append values (and prob better for mem management)
	// this means there will be 16 columns, one for each track
	std::vector<std::vector<float>> keyframes;
	float prevSaveVoltage = 0.f;
	float prevStartVoltage = 0.f;
	float prevStopVoltage = 0.f;

	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		START_INPUT,
		STOP_INPUT,
		SAVE_INPUT,
		_1_INPUT,
		_2_INPUT,
		_3_INPUT,
		_4_INPUT,
		_5_INPUT,
		_6_INPUT,
		_7_INPUT,
		_8_INPUT,
		_9_INPUT,
		_10_INPUT,
		_11_INPUT,
		_12_INPUT,
		_13_INPUT,
		_14_INPUT,
		_15_INPUT,
		_16_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		FRAME_START_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		FRAME_INDICATOR_LIGHT,
		LIGHTS_LEN
	};

	ToKeyframes() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(START_INPUT, "");
		configInput(STOP_INPUT, "");
		configInput(SAVE_INPUT, "");
		configInput(_1_INPUT, "");
		configInput(_2_INPUT, "");
		configInput(_3_INPUT, "");
		configInput(_4_INPUT, "");
		configInput(_5_INPUT, "");
		configInput(_6_INPUT, "");
		configInput(_7_INPUT, "");
		configInput(_8_INPUT, "");
		configInput(_9_INPUT, "");
		configInput(_10_INPUT, "");
		configInput(_11_INPUT, "");
		configInput(_12_INPUT, "");
		configInput(_13_INPUT, "");
		configInput(_14_INPUT, "");
		configInput(_15_INPUT, "");
		configInput(_16_INPUT, "");
		configOutput(FRAME_START_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {

		// activate recording if the RECORD input is triggered
		if(prevStartVoltage == 0.f && inputs[START_INPUT].getVoltage() > prevStartVoltage){
			recordingActive = true;
			startFrame = args.frame;
		}
		prevStartVoltage = inputs[START_INPUT].getVoltage();

		// deactivate recording and clear keyframes of the STOP input is triggered
		if(prevStopVoltage == 0.f && inputs[STOP_INPUT].getVoltage() > prevStopVoltage){
			keyframes.clear();
			recordingActive = false;
		}
		prevStopVoltage = inputs[STOP_INPUT].getVoltage();
		
		
		int framesInKeyframe = (int64_t)args.sampleRate / keyframeRate; //calculated every frame bc these values can change during execution

		// use the frame number and sample rate to determine the current time
		// determine if this frame is the frame when a keyframe needs to be saved based on keyframeRate
		if(recordingActive && (args.frame - startFrame) % framesInKeyframe == 0) {
			// output the value of the keyframe
			// converts this frame to a value between 0..1 depending on the frame it is in this second 1/24, 2/24, ...
			outputs[FRAME_START_OUTPUT].setVoltage( (float)((args.frame / framesInKeyframe) % keyframeRate) / (float)(keyframeRate) );
			
			std::vector<float> thisKeyframe = {
				inputs[_1_INPUT].getVoltage(),
				inputs[_2_INPUT].getVoltage(),
				inputs[_3_INPUT].getVoltage(),
				inputs[_4_INPUT].getVoltage(),
				inputs[_5_INPUT].getVoltage(),
				inputs[_6_INPUT].getVoltage(),
				inputs[_7_INPUT].getVoltage(),
				inputs[_8_INPUT].getVoltage(),
				inputs[_9_INPUT].getVoltage(),
				inputs[_10_INPUT].getVoltage(),
				inputs[_11_INPUT].getVoltage(),
				inputs[_12_INPUT].getVoltage(),
				inputs[_13_INPUT].getVoltage(),
				inputs[_14_INPUT].getVoltage(),
				inputs[_15_INPUT].getVoltage(),
				inputs[_16_INPUT].getVoltage()
			};

			keyframes.push_back(thisKeyframe);

			DEBUG("number of keyframes is %ld", keyframes.size());
		}

		// asynchronously save the keyframes to disk as a CSV file upon trigger
		if(prevSaveVoltage == 0.f && inputs[SAVE_INPUT].getVoltage() > prevSaveVoltage){
			std::async(saveKeyframesToCSV, keyframes);
			keyframes.clear();
		}
		prevSaveVoltage = inputs[SAVE_INPUT].getVoltage();
	}
};


struct ToKeyframesWidget : ModuleWidget {
	ToKeyframesWidget(ToKeyframes* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/ToKeyframes.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.947, 19.138)), module, ToKeyframes::START_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.626, 19.143)), module, ToKeyframes::STOP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.308, 19.141)), module, ToKeyframes::SAVE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.122, 42.757)), module, ToKeyframes::_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.459, 42.757)), module, ToKeyframes::_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.794, 42.758)), module, ToKeyframes::_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.132, 42.759)), module, ToKeyframes::_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.12, 55.112)), module, ToKeyframes::_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.458, 55.112)), module, ToKeyframes::_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.797, 55.11)), module, ToKeyframes::_7_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.134, 55.108)), module, ToKeyframes::_8_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.121, 67.462)), module, ToKeyframes::_9_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.459, 67.461)), module, ToKeyframes::_10_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.795, 67.461)), module, ToKeyframes::_11_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.132, 67.462)), module, ToKeyframes::_12_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.122, 79.805)), module, ToKeyframes::_13_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.458, 79.804)), module, ToKeyframes::_14_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.796, 79.804)), module, ToKeyframes::_15_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.133, 79.804)), module, ToKeyframes::_16_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.627, 101.444)), module, ToKeyframes::FRAME_START_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(34.141, 98.447)), module, ToKeyframes::FRAME_INDICATOR_LIGHT));
	}
};


Model* modelToKeyframes = createModel<ToKeyframes, ToKeyframesWidget>("ToKeyframes");
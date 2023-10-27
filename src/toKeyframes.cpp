#include "plugin.hpp"


struct ToKeyframes : Module {
	int64_t keyframeRate = 2;

	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		TRACKONE_INPUT,
		TRACKTWO_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		DEBUG_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	ToKeyframes() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(TRACKONE_INPUT, "");
		configInput(TRACKTWO_INPUT, "");
		configOutput(DEBUG_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
		// use the frame number and sample rate to determine the current time
		// determine if this frame is the frame when a keyframe needs to be saved based on keyframeRate
		int framesInKeyframe = (int64_t)args.sampleRate / keyframeRate;
		if(args.frame % framesInKeyframe == 0){
			// output the value of the keyframe
			// converts this frame to a value between 0..1 depending on the frame it is in this second 1/24, 2/24, ...
			outputs[DEBUG_OUTPUT].setVoltage( (float)((args.frame / framesInKeyframe) % keyframeRate) / (float)(keyframeRate) );
		}
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

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.954, 56.419)), module, ToKeyframes::TRACKONE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.728, 66.185)), module, ToKeyframes::TRACKTWO_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.003, 83.231)), module, ToKeyframes::DEBUG_OUTPUT));
	}
};


Model* modelToKeyframes = createModel<ToKeyframes, ToKeyframesWidget>("ToKeyframes");
#include "plugin.hpp"


struct ToKeyframes : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		TRACKONE_INPUT,
		TRACKTWO_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	ToKeyframes() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(TRACKONE_INPUT, "");
		configInput(TRACKTWO_INPUT, "");
	}

	void process(const ProcessArgs& args) override {
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
	}
};


Model* modelToKeyframes = createModel<ToKeyframes, ToKeyframesWidget>("ToKeyframes");
#include "pop-mass-edit.hpp"
#include "auto-complete.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <string>
#include <vector>
#include "../map/unordered_dense.h"
#include "../editor-state/editor-state.hpp"

namespace widgets {
namespace modal {
void pop_query(state::layers_stack& layers) {
	if (ImGui::BeginPopupModal("Mass pop operation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("This operation cannot be undone.");
		ImGui::Separator();
		ImGui::Text("Choose pops you want to change");

		auto poptypes = layers.retrieve_poptypes();
                auto religions = layers.retrieve_religions();

		static ankerl::unordered_dense::map<std::string, bool> selected_pop_types {};

		static std::string selected_religion;
		static bool ignore_religion;
		ImGui::Checkbox("Ignore religion", &ignore_religion);
		widgets::auto_complete::religion(layers, selected_religion);

		static std::string selected_culture;
		static bool ignore_culture;
		ImGui::Checkbox("Ignore culture", &ignore_culture);
		widgets::auto_complete::culture(layers, selected_culture);

		auto pop_types = layers.retrieve_poptypes();

		for (auto& pt:pop_types) {
			if (!selected_pop_types.contains(pt)) {
				selected_pop_types[pt] = true;
			}
			ImGui::Checkbox(pt.c_str(), &selected_pop_types[pt]);
		}


		ImGui::Separator();
		static int selection = 0;
		static std::string changes[] = {"Set to zero", "Resize", "Convert"};

		if (ImGui::BeginCombo("Describe the change", changes[selection].c_str())) {
			for (int n = 0; n <3; n++) {
				if (ImGui::Selectable(changes[n].c_str(), n == selection)) {
					selection = n;
				}
			}
			ImGui::EndCombo();
		}

		auto dates = layers.get_available_dates();
		static int selected_date = dates[0];

		if (ImGui::BeginCombo("Date", std::to_string(selected_date / 31 / 12).c_str())) {
			for (auto date : dates) {
				const bool is_selected = (selected_date == date);
				if (ImGui::Selectable(std::to_string(date / 31 / 12).c_str(), is_selected)) {
					selected_date = date;
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}


		static bool calculated_stats;
		static float total_size;
		static int count;

		if (ImGui::Button("Calculate stats")) {
			for (auto& [v2id, value] : *layers.get_v2ids()) {
				auto pops = layers.get_pops(v2id, selected_date);
				if (!pops) continue;
				for (auto& pop : *pops) {
					if (pop.culture != selected_culture && !ignore_culture) continue;
					if (pop.religion != selected_religion && !ignore_religion) continue;
					if (!selected_pop_types[pop.poptype]) continue;

					total_size += pop.size;
					count += 1;
				}
			}
			calculated_stats = true;
		}
		if (calculated_stats) {
			ImGui::Text("%f people and %d pops were selected during last recalculation", total_size, count);
		}


		if (selection == 0) {
			if (ImGui::Button("Confirm deletion")) {
				for (auto& [v2id, value] : *layers.get_v2ids()) {
					bool exist = false;
					{
						// copy if exist
						auto pops = layers.get_pops(v2id, selected_date);
						if (!pops) continue;
						for (auto& pop : *pops) {
							if (pop.culture != selected_culture && !ignore_culture) continue;
							if (pop.religion != selected_religion && !ignore_religion) continue;
							if (!selected_pop_types[pop.poptype]) continue;
							exist = true;
						}
						if (exist) {
							layers.copy_pops_data_to_current_layer(v2id, selected_date);
						}
					}
					if (exist) {
						auto pops = layers.get_pops(v2id, selected_date);
						for (auto& pop : *pops) {
							if (pop.culture != selected_culture && !ignore_culture) continue;
							if (pop.religion != selected_religion && !ignore_religion) continue;
							if (!selected_pop_types[pop.poptype]) continue;

							pop.size = 0;
						}
					}
				}
			}
		}

		if (selection == 1) {
			static float mult;
			ImGui::InputFloat("Multiply by", &mult);
			if (ImGui::Button("Confirm multiplication")) {
				for (auto& [v2id, value] : *layers.get_v2ids()) {
					bool exist = false;
					{
						// copy if exist
						auto pops = layers.get_pops(v2id, selected_date);
						if (!pops) continue;
						for (auto& pop : *pops) {
							if (pop.culture != selected_culture && !ignore_culture) continue;
							if (pop.religion != selected_religion && !ignore_religion) continue;
							if (!selected_pop_types[pop.poptype]) continue;
							exist = true;
						}
						if (exist) {
							layers.copy_pops_data_to_current_layer(v2id, selected_date);
						}
					}
					if (exist) {
						auto pops = layers.get_pops(v2id, selected_date);
						for (auto& pop : *pops) {
							if (pop.culture != selected_culture && !ignore_culture) continue;
							if (pop.religion != selected_religion && !ignore_religion) continue;
							if (!selected_pop_types[pop.poptype]) continue;

							pop.size *= mult;
						}
					}
				}
			}
		}

		if (selection == 2) {
			ImGui::PushID("target");
			static std::string target_religion;
			widgets::auto_complete::religion(layers, target_religion);
			static std::string target_culture;
			widgets::auto_complete::culture(layers, target_culture);
			ImGui::PopID();

			static float convertion_rate;
			ImGui::SliderFloat("Converted ratio", &convertion_rate, 0.f, 1.f);

			if (ImGui::Button("Confirm conversion")) {
				for (auto& [v2id, value] : *layers.get_v2ids()) {

					bool exist = false;
					{
						// copy if exist
						auto pops = layers.get_pops(v2id, selected_date);
						if (!pops) continue;
						for (auto& pop : *pops) {
							if (pop.culture != selected_culture && !ignore_culture) continue;
							if (pop.religion != selected_religion && !ignore_religion) continue;
							if (!selected_pop_types[pop.poptype]) continue;
							exist = true;
						}
						if (exist) {
							layers.copy_pops_data_to_current_layer(v2id, selected_date);
						}
					}

					if (exist) {
						auto pops = layers.get_pops(v2id, selected_date);
						std::vector<game_definition::pop_history> pops_to_add;
						for (auto& pop : *pops) {
							if (pop.culture != selected_culture && !ignore_culture) continue;
							if (pop.religion != selected_religion && !ignore_religion) continue;
							if (!selected_pop_types[pop.poptype]) continue;

							game_definition::pop_history new_pop = pop;
							new_pop.culture = target_culture;
							new_pop.religion = target_religion;
							new_pop.size = pop.size * convertion_rate;

							pop.size = pop.size * (1.f - convertion_rate);

							pops_to_add.push_back({
								new_pop
							});
						}
						for (auto& pop_to_add : pops_to_add) {
							bool already_exists = false;
							for (auto & candidate : *pops) {
								if (
									candidate.culture == pop_to_add.culture
									&& candidate.poptype == pop_to_add.poptype
									&& candidate.religion == pop_to_add.religion
								) {
									already_exists = true;
									candidate.size += pop_to_add.size;
									break;
								}
							}
							if (!already_exists) pops->push_back(pop_to_add);
						}
					}
				}
			}
		}

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
}
}
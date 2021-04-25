<template>
  <v-container>
    <v-row v-if="filePath == null">
      <v-col>
        <v-file-input
          accept=".jsonc"
          label="Select a Decompiler Configuration (.jsonc) File"
          @change="fileInputChanged"
        ></v-file-input>
      </v-col>
    </v-row>
    <div v-if="fileContent != null">
      <v-row>
        <v-col align="center">
          <h2>
            Config File Loaded
          </h2>
          {{ filePath }}
        </v-col>
      </v-row>
      <v-row>
        <v-col>
          <v-expansion-panels multiple>
            <v-expansion-panel
              v-for="(settings, heading) in contentToRender"
              :key="heading"
            >
              <v-expansion-panel-header>
                {{ heading }}
              </v-expansion-panel-header>
              <v-expansion-panel-content>
                <v-container>
                  <v-row
                    v-for="(metadata, settingName) in settings"
                    align="center"
                    :key="settingName"
                  >
                    <!-- Checkboxes -->
                    <v-col v-if="metadata.type == 'bool'" cols="4">
                      <v-checkbox
                        v-model="fileContent[settingName]"
                        :label="metadata.label"
                      ></v-checkbox>
                    </v-col>
                    <v-col v-if="metadata.type == 'bool'" cols="8">
                      {{ metadata.help }}
                    </v-col>
                    <!-- Numeric Entry -->
                    <v-col cols="12" v-if="metadata.type == 'number'">
                      <v-text-field
                        v-model="fileContent[settingName]"
                        type="number"
                        :label="metadata.label"
                        :hint="metadata.help"
                        persistent-hint
                      ></v-text-field>
                    </v-col>

                    <!-- String List -->
                    <v-col cols="12" v-if="metadata.type == 'list-string'">
                      <v-combobox
                        required
                        clearable
                        multiple
                        chips
                        deletable-chips
                        :label="metadata.label"
                        :hint="metadata.help"
                        persistent-hint
                        :items="fileContent[settingName]"
                        v-model="fileContent[settingName]"
                      ></v-combobox>
                    </v-col>

                    <!-- References Files (opens in text editor) -->
                    <v-col
                      cols="6"
                      v-if="metadata.type == 'project-relative-file-path'"
                    >
                      {{ metadata.label }} -
                      <span class="font-weight-thin"
                        ><em>{{ metadata.help }}</em></span
                      >
                    </v-col>
                    <v-file-input
                      v-if="metadata.type == 'project-relative-file-path'"
                      accept=".jsonc"
                      label="File Path (Relative to jak-project dir)"
                      :value="getRelativeFilePath(settingName)"
                      @change="relativeFilePathChanged(settingName)"
                    ></v-file-input>
                    <v-col
                      cols="auto"
                      v-if="metadata.type == 'project-relative-file-path'"
                    >
                      <v-btn @click="openTextFile(fileContent[settingName])">
                        Open in Text Editor
                      </v-btn>
                    </v-col>
                  </v-row>
                </v-container>
              </v-expansion-panel-content>
            </v-expansion-panel>
          </v-expansion-panels>
        </v-col>
      </v-row>
      <v-row justify="center">
        <v-col cols="auto" align="center">
          <v-btn>
            Save
          </v-btn>
        </v-col>
        <v-col cols="auto" align="center">
          <v-btn>
            Save As
          </v-btn>
        </v-col>
      </v-row>
    </div>
  </v-container>
</template>

<script>
const path = require("path");
const shell = require("electron").shell;
const fs = require("fs");
const decomment = require("decomment");

export default {
  name: "DecompilerConfigs",
  data: () => ({
    current: new File(["foo"], "foo.txt", {
      type: "text/plain"
    }),
    filePath: null,
    fileContent: null,
    contentToRender: {},
    // TODO move to another file
    contentOrganizer: {
      "General Settings": {
        game_version: {
          type: "number",
          label: "Game Version",
          help:
            "Specifies which version of the particular game this configuration pertains to.  Different versions may have different files / etc."
        }
      },
      "Decompilation Settings": {
        analyze_functions: {
          type: "bool",
          label: "Analyze Functions",
          help: "Whether or not the decompiler should analyze functions TODO"
        },
        analyze_expressions: {
          type: "bool",
          label: "Analyze Expressions",
          help:
            "Whether or not the decompiler should do it's expression pass -- possibly re-writing the original output into a more OpenGOAL format."
        },
        function_type_prop: {
          type: "bool",
          label: "Function Type Property",
          help: "Attempts to identify the function type? TODO"
        },
        insert_lets: {
          type: "bool",
          label: "Insert (let)s",
          help:
            "Whether or not the decompiler should attempt to insert additional `let` forms"
        },
        run_ir2: {
          type: "bool",
          label: "Run with V2 Intermittent Representation",
          help: "Use the new IR representation"
        },
        disassemble_objects_without_functions: {
          type: "bool",
          label: "Disassemble Objects Without Functions",
          help:
            "Skips printing disassembly of object with functions, as these are usually large (~1 GB) and not interesting yet."
        },
        process_tpages: {
          type: "bool",
          label: "Process Texture Pages",
          help: "Whether or not the decompiler should process texture pages"
        },
        process_game_text: {
          type: "bool",
          label: "Process Game Text",
          help: "Whether or not the decompiler should process game text"
        },
        process_game_count: {
          type: "bool",
          label: "Process Game Count",
          help: "TODO"
        }
      },
      "Input Options": {
        dgo_names: {
          type: "list-string",
          label: "DGO Names",
          help:
            "Which CGO/DGO files from the ISO should be attempted to be disassembled.  The order here _may_ matter! Kernel and Game should go first."
        },
        object_file_names: {
          type: "list-string",
          label: "Object File Names",
          help: "Which files from the ISO are simplistic object files. TODO"
        },
        str_file_names: {
          type: "list-string",
          label: "STR File Names",
          help: "Unordered list of .STR files from the ISO. TODO"
        },
        allowed_objects: {
          type: "list-string",
          label: "Allowed Objects",
          help: "TODO - better description"
        }
      },
      "Output Options": {
        write_disassembly: {
          type: "bool",
          label: "Write Disassembly",
          help:
            "Whether or not the decompiler should write out the disassembly in addition to the OpenGOAL code"
        },
        write_hex_near_instructions: {
          type: "bool",
          label: "Write HEX near Instructions",
          help: "TODO?"
        },
        dump_objs: {
          type: "bool",
          label: "Dump Objects",
          help: "TODO?"
        },
        write_func_json: {
          type: "bool",
          label: "Write Function JSON",
          help: "TODO?"
        },
        write_hexdump: {
          type: "bool",
          label: "Write Hexdump",
          help: "To write out data of each object file"
        },
        write_hexdump_on_v3_only: {
          type: "bool",
          label: "Write Hexdump on v3 files only",
          help:
            "To write out hexdump on the v3 files only, to avoid the huge level data files. Only if write_hexdump is true."
        },
        write_scripts: {
          type: "bool",
          label: "Write Scripts",
          help:
            "To write out `scripts`, which are currently just all the linked lists found. mostly a jak 2/3 thing."
        }
      },
      "Referenced Files": {
        // TODO - make these editable in the GUI as well
        type_casts_file: {
          type: "project-relative-file-path",
          label: "Type Casts File",
          help: "Manually specify the types of symbols"
        },
        anonymous_function_types_file: {
          type: "project-relative-file-path",
          label: "Anonymous Function Types File",
          help: "Manually specify the types of anonymous functions"
        },
        var_names_file: {
          type: "project-relative-file-path",
          label: "Variable Names File",
          help: "Manually specify the names of variables / symbols"
        },
        label_types_file: {
          type: "project-relative-file-path",
          label: "Label Types File",
          help: "Manually specify the types of labels"
        },
        stack_vars_file: {
          type: "project-relative-file-path",
          label: "Stack Variables File",
          help:
            "Manually specify which variables are to be placed on the stack TODO?"
        },
        obj_file_name_map_file: {
          type: "project-relative-file-path",
          label: "Object File-Name Map File",
          help:
            "optional: a predetermined object file name map from a file. Useful if you want to run only on some DGOs but have consistent names"
        }
      },
      "Manual Declarations": {
        types_with_bad_inspect_methods: {
          type: "list-string",
          label: "Types with Bad Inspect Methods",
          help:
            "List of types which have bad inspect methods so they can be handled... TODO?"
        },
        no_type_analysis_functions_by_name: {
          type: "list-string",
          label: "No Type Analysis Functions by Name",
          help: "List of functions to skip type analysis on"
        },
        asm_functions_by_name: {
          type: "list-string",
          label: "ASM Functions by Name",
          help: "Specifies which functions are inline assembly"
        },
        pair_functions_by_name: {
          type: "list-string",
          label: "Pair Functions by Name",
          help: "TODO"
        }
      }
    }
  }),
  components: {},
  methods: {
    fileInputChanged: async function(file) {
      if (file == null) {
        this.filePath == null;
        this.fileContent == null;
      } else {
        this.filePath = file.path;
        this.fileContent = JSON.parse(
          decomment(
            fs.readFileSync(this.filePath, { encoding: "utf8", flag: "r" })
          )
        );

        for (const [key, val] of Object.entries(this.contentOrganizer)) {
          var currentSegment = {};
          for (const [setting, metadata] of Object.entries(val)) {
            if (setting in this.fileContent) {
              currentSegment[setting] = metadata;
            }
          }
          if (Object.keys(currentSegment).length !== 0) {
            this.contentToRender[key] = currentSegment;
          }
        }
      }
    },
    openTextFile: async function(relativePath) {
      var basePath = this.filePath.split("jak-project")[0];
      shell.openPath(path.join(basePath, "jak-project", relativePath));
    },
    relativeFilePathChanged: async function(file, settingName) {
      if (file == null) {
        return;
      } else {
        var relativePath = this.file.path.split("jak-project")[0];
        this.fileContent[settingName] = relativePath;
      }
    },
    getRelativeFilePath: function(settingName) {
      var basePath = this.filePath.split("jak-project")[0];
      var fullPath = path.join(
        basePath,
        "jak-project",
        this.fileContent[settingName]
      );
      return new File([], path.basename(fullPath), {
        type: "text/plain"
      });
    }
  }
};
</script>

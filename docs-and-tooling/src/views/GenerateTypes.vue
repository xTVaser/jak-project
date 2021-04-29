<template>
  <v-container>
    <v-row v-if="filePath == null">
      <v-col>
        <v-file-input
          accept=".json"
          label="Select a Build Order (.json) File"
          @change="fileInputChanged"
        ></v-file-input>
      </v-col>
    </v-row>
    <v-row v-if="successMessage != null">
      <v-col>
        <h2>{{ successMessage }}</h2>
      </v-col>
    </v-row>
    <v-row v-if="errorMessage != null">
      <v-col>
        <p>
          {{ errorMessage }}
        </p>
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
const path = require("path");
const fs = require("fs");
const decomment = require("decomment");

export default {
  name: "GenerateTypes",
  data: () => ({
    current: new File(["foo"], "foo.txt", {
      type: "text/plain"
    }),
    filePath: null,
    fileContent: null,
    successMessage: null,
    errorMessage: null
  }),
  components: {},
  methods: {
    fileInputChanged: async function(file) {
      if (file == null) {
        this.filePath == null;
        this.fileContent == null;
      } else {
        try {
          this.filePath = file.path;
          this.fileContent = JSON.parse(
            decomment(
              fs.readFileSync(this.filePath, { encoding: "utf8", flag: "r" })
            )
          );

          var outputContent = "";

          for (var i = 0; i < this.fileContent.length; i++) {
            const obj = this.fileContent[i];
            const dirPath = obj.dir;
            for (var j = 0; j < obj.files.length; j++) {
              const file = obj.files[j];
              outputContent += `;; ---\n;; --- ${dirPath}/${file} ---\n;; ---\n`
              outputContent += fs.readFileSync(path.join(path.dirname(this.filePath), dirPath, file));
            }
          }

          fs.writeFileSync(path.join(path.dirname(this.filePath), "all-types.gc"), outputContent);
          this.successMessage = `Successfully wrote combined all-types file to - ${path.join(path.dirname(this.filePath), "all-types.gc")}`;
          this.errorMessage = null;
          this.filePath = null;
        } catch (err) {
          this.errorMessage = err;
          this.successMessage = null;
          this.filePath = null;
          this.fileContent = null;
        }
      }
    },
  }
};
</script>

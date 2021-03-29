<template>
  <v-card>
    <v-container>
    <v-file-input accept=".jsonc" label="Browse for JSON File" @change="loadTextFromFile"></v-file-input>
    </v-container>
  </v-card>
</template>

<script>
export default {
  methods: {
    loadTextFromFile(ev) {
      const file = ev.target.files[0];
      const reader = new FileReader();

      reader.onload = e => this.$emit("load", e.target.result);
      reader.readAsText(file);
    }
  }
};
</script>
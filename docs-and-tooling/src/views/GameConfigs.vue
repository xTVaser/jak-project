<template>
  <!-- Allow splitting up all-types.gc into multiple files and re-compiling the new version -->
    <!-- Detect function name conflicts / use the ACE editor -->
    <!-- Allow jumpin -->
  <!--  Allow modifying the existing config files in a more organized manner -->
  <v-container>
    <v-card>
      <FileReader/>
    </v-card>
  </v-container>

</template>

<script>
import FileReader from "@/components/FileReader";

export default {
  name: "GameConfigs",
  data: () => ({ text: "" }),
  components: {
    FileReader
  }
};
</script>
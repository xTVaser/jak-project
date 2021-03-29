<template>
  <v-app>
    <v-app-bar app color="primary" dark>
      <div class="d-flex align-center">
        <v-btn to="/" text>
          <v-img
            alt="OpenGOAL Logo"
            class="shrink mr-2"
            contain
            src="logo-text-colored.png"
            transition="scale-transition"
            width="150"
          />
        </v-btn>
      </div>

      <v-spacer></v-spacer>

      <v-btn to="/docs" text>
        <span class="mr-2">API Docs</span>
      </v-btn>
      <v-btn to="/configs" text>
        <span class="mr-2">Decompiler Configs</span>
      </v-btn>

      <v-spacer></v-spacer>

      <v-btn
        href="https://github.com/water111/jak-project"
        target="_blank"
        text
      >
        <span class="mr-2">Repository</span>
        <v-icon>mdi-github</v-icon>
      </v-btn>
    </v-app-bar>

    <v-main>
      <router-view />
    </v-main>
  </v-app>
</template>

<script>
export default {
  name: "App",

  components: {},

  data: () => ({
    //
  })
};
</script>

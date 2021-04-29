module.exports = {
  transpileDependencies: [
    'vuetify'
  ],
  pluginOptions: {
    electronBuilder: {
      nodeIntegration: true
    }
  },
  publicPath: process.env.NODE_ENV === 'production'
    ? '/jak-project/'
    : '/'
}

const fs = require('fs')
const path = require('path')

const gulp = require('gulp-param')(require('gulp'), process.argv)
const htmlmin = require('gulp-htmlmin')
const sass = require('gulp-sass')
const autoprefixer = require('gulp-autoprefixer')
const cssnano = require('gulp-cssnano')
const rename = require('gulp-rename')
const gzip = require('gulp-gzip')
const imagemin = require('gulp-imagemin')
const crx = require('gulp-crx-pack')
const webpack = require('webpack')

const ERROR_EVENT = 'error'

const packageInfo = require(`./package.json`)

const sourceDir = path.join(__dirname, 'src')
const buildDir = path.join(__dirname, 'build', packageInfo.version)

function errorHandler () {
  console.error(Array.prototype.slice.call(arguments))

  this.emit('end')
}

/**
 * Copy meta.
 */
gulp.task('meta', function () {
  return gulp.src([ './LICENSE', sourceDir + '/manifest.json' ])
    // Specify the output destination
    .pipe(gulp.dest(buildDir))
    // Report errors
    .on(ERROR_EVENT, errorHandler)
})

/**
 * Copy messages.
 */
gulp.task('messages', function () {
  // Define the target directory
  const targetDir = `${buildDir}/_locales`

  return gulp.src(`${sourceDir}/_locales/**/messages.json`)
    // Specify the output destination
    .pipe(gulp.dest(targetDir))
    // Report errors
    .on(ERROR_EVENT, errorHandler)
})

/**
 * Gzip JSON data.
 */
gulp.task('data', function () {
  // Define the target directory
  const targetDir = `${buildDir}/resources/data`

  return gulp.src(`${sourceDir}/resources/data/*`)
    .pipe(gzip({
      preExtension: 'gz',
      gzipOptions: {
        level: 9
      }
    }))
    // Specify the output destination
    .pipe(gulp.dest(targetDir))
    // Report errors
    .on(ERROR_EVENT, errorHandler)
})

/**
 * Minify images.
 */
gulp.task('images', function () {
  // Define the target directory
  const targetDir = `${buildDir}/resources`

  return gulp.src(`${sourceDir}/resources/**/*.png`)
    .pipe(imagemin({
      optimizationLevel: 7
    }))
    // Specify the output destination
    .pipe(gulp.dest(targetDir))
    // Report errors
    .on(ERROR_EVENT, errorHandler)
})

/**
 * Create and minify HTML templates.
 */
gulp.task('templates', function () {
  return gulp.src(`${sourceDir}/*.html`)
    .pipe(htmlmin({
      collapseWhitespace: true,
      quotes: true
    }))
    // Specify the output destination
    .pipe(gulp.dest(buildDir))
    // Report errors
    .on(ERROR_EVENT, errorHandler)
})

/**
 * Compile style sheets.
 */
gulp.task('sass', function (debug) {
  // Define the target directory
  const targetDir = `${buildDir}/resources/styles`

  const file = `${sourceDir}/resources/styles/styles.scss`

  if (debug) {
    return compileStyles(file)
    // Specify the output destination
      .pipe(gulp.dest(targetDir))
      // Report errors
      .on(ERROR_EVENT, errorHandler)
  } else {
    return compileStyles(file)
    // Minify the output
      .pipe(cssnano())
      // Specify the output destination
      .pipe(gulp.dest(targetDir))
      // Report errors
      .on(ERROR_EVENT, errorHandler)
  }
})

/**
 * Copy webfonts.
 */
gulp.task('webfonts', function () {
  // Define the target directory
  const targetDir = `${buildDir}/resources/webfonts`

  return gulp.src(`${sourceDir}/resources/webfonts/**`)
    // Specify the output destination
    .pipe(gulp.dest(targetDir))
    // Report errors
    .on(ERROR_EVENT, errorHandler)
})


/**
 * Bundle modules.
 */
gulp.task('bundle', function (debug, callback) {
  // Define the target directory
  const targetDir = `${buildDir}/lib`

  const plugins = [
    new webpack.optimize.OccurenceOrderPlugin(),
    new webpack.NoErrorsPlugin()
  ];

  if (!debug) {
    //plugins.push(new webpack.optimize.UglifyJsPlugin());
  }

  webpack({
    entry: `${sourceDir}/lib/process.js`,
    output: {
      path: targetDir,
      filename: 'process.js'
    },
    compilerOptions: {
      progress: false,
      json: false,
      memoryFs: false
    },
    cache: false,
    plugins: plugins,
    module: {
      loaders: [
        { test: /\.jsx?$/, exclude: /node_modules/, loader: 'babel-loader' }
      ]
    }
  }, function (error) {
    if(error) throw new Error('webpack', error.toString())

    callback()
  })
})

/**
 * Copy NaCl modules.
 */
gulp.task('nacl', function (debug) {
  // Define the target directory
  const targetDir = `${buildDir}/nacl`

  return gulp.src([
    `!${__dirname}/clang-newlib/${debug ? 'Debug' : 'Release'}/*_unstripped_*.nexe`,
    `${__dirname}/clang-newlib/${debug ? 'Debug' : 'Release'}/*.nexe`,
    `${__dirname}/clang-newlib/${debug ? 'Debug' : 'Release'}/*.nmf`
  ])
    // Specify the output destination
    .pipe(gulp.dest(function(file) {
      file.path = file.base + path.basename(file.path);
      return targetDir
    }))
    // Report errors
    .on(ERROR_EVENT, errorHandler)
})

/**
 * Package the extension for Chrome.
 */
gulp.task('package', [ 'meta', 'data', 'messages', 'images', 'templates', 'sass', 'webfonts', 'bundle', 'nacl' ], function (debug) {
  const packageName = `${packageInfo.name}-${packageInfo.version}${debug ? '-debug' : ''}`

  return gulp.src(buildDir)
    .pipe(crx({
      privateKey: fs.readFileSync('./chrome.pem', 'utf8'),
      filename: `chrome-${packageName}.crx`
    }))
    // Specify the output destination
    .pipe(gulp.dest(__dirname))
})

/**
 * Main task.
 */
gulp.task('build', [ 'package' ])

function compileStyles (file) {
  return gulp.src(file)
    .pipe(sass())
    // Parse CSS and add vendor prefixes
    .pipe(autoprefixer({
      browsers: [
        'Chrome > 42'
      ]
    }))
    .pipe(rename('styles.css'))
}
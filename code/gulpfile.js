/*

ESP8266 file system builder

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// -----------------------------------------------------------------------------
// File system builder
// -----------------------------------------------------------------------------

const fs = require('fs');
const gulp = require('gulp');
const plumber = require('gulp-plumber');
const htmlmin = require('gulp-htmlmin');
const cleancss = require('gulp-clean-css');
const uglify = require('gulp-uglify');
const gzip = require('gulp-gzip');
const del = require('del');
const useref = require('gulp-useref');
const gulpif = require('gulp-if');
const inline = require('gulp-inline');
const inlineImages = require('gulp-css-base64');
const favicon = require('gulp-base64-favicon');
const ini = require('ini');
const util = require('gulp-util');
const colors = require('colors');
const format = require('util').format;
const exec = require('child_process').exec;
const spawn = require( 'child_process' ).spawn;

const dataFolder = 'espurna/data/';
const boards = {
    'esp01_1m': 'generic',
    'd1_mini': 'd1_mini'
};
const flashsizes = {
    'esp01_1m': '1M64',
    'd1_mini': '4M3M'
};
const config = ini.parse(fs.readFileSync('./platformio.ini', 'utf-8'));
const buildCommand = '%s/arduino-builder -compile -hardware %s/hardware -hardware %s -tools %s/tools-builder -tools %s/hardware/tools/avr -tools %s -built-in-libraries %s/libraries -libraries .piolibdeps -fqbn=esp8266:esp8266:%s:CpuFrequency=80,UploadSpeed=921600,FlashSize=%s -build-path %s -warnings=none %s'

var getEnvironments = function() {
    var environments = [];
    Object.keys(config).forEach(function(key) {
        if ((/^env:/).test(key) && (/-debug$/).test(key)) {
            environments.push(key.substring(4,key.length-6));
        };
    });
    return environments;
}

var runCmd = function(cmd, args, callBack) {
    var spawn = require('child_process').spawn;
    var child = spawn(cmd, args);
    var resp = "";
    child.stdout.on('data', function (buffer) { resp += buffer.toString() });
    child.stdout.on('end', function() { callBack(resp) });
}

var buildEnvironment = function(environment) {

    console.log(colors.cyan("Building environment '" + environment + "'"));

    var data = config['env:' + environment + '-debug'];
    var board = boards[data.board];
    var flashsize = flashsizes[data.board];

    var command = format(buildCommand,
        config.arduino.ide_path,
        config.arduino.ide_path,
        config.arduino.arduino_packages,
        config.arduino.ide_path,
        config.arduino.ide_path,
        config.arduino.arduino_packages,
        config.arduino.ide_path,
        board,
        flashsize,
        config.arduino.build_path,
        config.arduino.sketch_folder
    );

    console.log(command);

    fs.rename(config.arduino.config_file, config.arduino.config_file + '.backup', function() {

        var wstream = fs.createWriteStream(config.arduino.config_file);
        data.build_flags.split(' ').forEach(function(flag) {
            if (flag.indexOf('-D') == 0) {
                wstream.write('#define ' + flag.substring(2) + '\n');
            }
        });
        wstream.end();

        exec(command, function(error, stdout, stderr) {
            console.log(stdout);
            if (error) console.log(colors.red(error));
            fs.rename(config.arduino.config_file + '.backup', config.arduino.config_file);
        });

    });

}

gulp.task('arduino', function() {

    var environments = getEnvironments();

    if (typeof util.env.all == 'undefined') {
        if (typeof util.env.e == 'undefined') {
            console.log("Usage: gulp arduino [--all | -e <environment>]");
            return;
        } else if (environments.indexOf(util.env.e) >= 0) {
            environments = [util.env.e];
        } else {
            console.log("Environment '"+ util.env.e + "' does not exist!");
            return;
        }
    }

    environments.forEach(buildEnvironment);

});

gulp.task('clean', function() {
    del([ dataFolder + '*']);
    return true;
});

gulp.task('files', ['clean'], function() {
    return gulp.src([
            'html/**/*.{jpg,jpeg,png,ico,gif}',
            'html/fsversion'
        ])
        .pipe(gulp.dest(dataFolder));
});

gulp.task('buildfs_embeded', ['buildfs_inline'], function() {

    var source = dataFolder + 'index.html.gz';
    var destination = dataFolder + '../static/index.html.gz.h';

    var wstream = fs.createWriteStream(destination);
    wstream.on('error', function (err) {
        console.log(err);
    });

    var data = fs.readFileSync(source);

    wstream.write('#define index_html_gz_len ' + data.length + '\n');
    wstream.write('const uint8_t index_html_gz[] PROGMEM = {')

    for (i=0; i<data.length; i++) {
        if (i % 1000 == 0) wstream.write("\n");
        wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
        if (i<data.length-1) wstream.write(',');
    }

    wstream.write('\n};')
    wstream.end();

});

gulp.task('buildfs_inline', ['clean'], function() {
    return gulp.src('html/*.html')
        .pipe(favicon())
        .pipe(inline({
            base: 'html/',
            js: uglify,
            css: [cleancss, inlineImages],
            disabledTypes: ['svg', 'img']
        }))
        .pipe(htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true
        }))
        .pipe(gzip())
        .pipe(gulp.dest(dataFolder));
})

gulp.task('buildfs_split', ['files'], function() {
    return gulp.src('html/*.html')
        .pipe(useref())
        .pipe(plumber())
        .pipe(gulpif('*.css', cleancss()))
        .pipe(gulpif('*.js', uglify()))
        .pipe(gulpif('*.html', htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true
        })))
        .pipe(gzip())
        .pipe(gulp.dest(dataFolder));
});

gulp.task('default', ['buildfs_embeded']);

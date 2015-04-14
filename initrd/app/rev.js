// Copyright 2014 Runtime.JS project authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

(function() {
  "use strict";

  function readLine(cb) {
    isolate.env.stdin({
      mode: 'line',
      onData: function(data) {
        cb(data.text);
      }
    });
  }

  function reverse(s) {
    var o = '';
    for (var i = s.length - 1; i >= 0; i--) {
      o += s[i];
    }
    return o;
  }

  var echo = function(data) {
    if (data) {
      console.log(reverse(data));
    }
    readLine(echo);
  }

  echo();

})();

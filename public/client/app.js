angular.module('app', [])
  .controller('myController', ['$scope', function ($scope) {

    $scope.items = [];
    var socket = io('ws://10.0.2.90:5000');
    socket.on('message', function(msg) {
      $scope.$apply(function(){
        if (msg.status == 'SUSPICIOUS') {
          $scope.items.push(msg);
        }
      });
    });

    socket.send({
      'expect_path': 'public/test_images/widget_common_normal',
      'target_path': 'public/test_images/widget_common_normal_scratch',
      //'expect_path': 'public/images',
      //'target_path': 'public/images2',
      'threshold': 5,
      'span': 10
    });

  }])
  .directive('flow', [function () {
    return {
      scope: {
        data: '='
      },
      restrict: 'E',
      link: function(scope, element) {

        scope.$watch('data', function(newval){

          var img = new Image();
          var expect_img = scope.data.expect_image.replace('public', 'http://10.0.2.90:5000');
          var target_img = scope.data.target_image.replace('public', 'http://10.0.2.90:5000');
          img.src = expect_img;
          img.onload = function () {

            var width = this.width;
            var height = this.height;
            var svg = d3.select(element[0])
              .append('svg')
              .attr('width', width)
              .attr('height', height);

            var svg_expect_img = svg.append("image")
              .attr("image-rendering", "optimizeQuality")
              .attr("x", '0')
              .attr("y", '0');

            svg_expect_img.attr('height', height)
               .attr('width', width)
               .attr('xlink:href', expect_img);

            var svg_target_img = svg.append("image")
              .attr("image-rendering", "optimizeQuality")
              .attr("x", '0')
              .attr("y", '0')
              .attr("opacity", '0.5');

            svg_target_img.attr('height', height)
               .attr('width', width)
               .attr('xlink:href', target_img);

            svg.selectAll('.vector-line')
              .data(scope.data.vector)
              .enter()
              .append('svg:line')
              .attr('class', 'vector-line')
              .attr({
                x1: function(d) {
                  return d.x;
                },
                y1: function(d) {
                  return d.y;
                },
                x2: function(d) {
                  return d.x + d.dx;
                },
                y2: function(d) {
                  return d.y + d.dy;
                },
                stroke: 'red'
              });
          };
        });

      }
    }
  }]);

angular.module('app', [])
  .controller('myController', ['$scope', function ($scope) {

    $scope.data = [];
    var socket = io('ws://10.0.2.90:5000');
    socket.on('message', function(data) {
      $scope.$apply(function(){
        $scope.data = data;
      });
    });

    socket.send({
      'expect_image': '/home/ikezoe_a/test_images/widget_common_normal',
      'target_image': '/home/ikezoe_a/test_images/widget_common_normal_scratch',
      'threshold': 5,
      'span': 10
    });

  }])
  .directive('flow', [function () {
    return {
      scope: {
        height: '=',
        width: '=',
        data: '='
      },
      restrict: 'E',
      link: function(scope, element) {
        var svg = d3.select(element[0])
          .append('svg')
          .attr('width', scope.width)
          .attr('height', scope.height);

        svg.append("image")
          .attr("xlink:href", "/images/customjsp1.png")
          .attr("width", scope.width)
          .attr("height", scope.height);


        scope.$watch('data', function(newval){
          console.time("link");
          svg.selectAll('.vector-line')
            .data(scope.data)
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
              stroke: 'black'
            });
          console.timeEnd("link");
        });

      }
    }
  }]);

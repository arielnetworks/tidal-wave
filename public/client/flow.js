angular.module('app')
  .directive('flow', [function () {
    return {
      scope: {
        data: '=',
        mode: '=',
        opacity: '='
      },
      templateUrl: 'flow.html',
      restrict: 'E',
      link: function (scope, element) {

        scope.$watch('mode', function (newval) {
          var imgAreaEl = element.find('.image-area')[0];
          d3.select(imgAreaEl)
            .select(".target-image")
            .style('display', newval === 'both' || newval === 'target' ? 'inline' : 'none');

          d3.select(imgAreaEl)
            .select(".expect-image")
            .style('display', newval === 'both' || newval === 'expect' ? 'inline' : 'none');

          d3.select(imgAreaEl)
            .select(".expect-image")
            .attr("opacity", newval === "expect" ? 1 : scope.opacity);
        });

        scope.$watch('opacity', function (newval) {
          var imgAreaEl = element.find('.image-area')[0];
          d3.select(imgAreaEl)
            .select(".expect-image")
            .attr("opacity", scope.mode === "expect" ? 1 : newval);
        });

        scope.$watch('data', function (newval) {

          var img = new Image();
          var expect_img = scope.data.expect_image.replace('public', 'http://10.0.2.90:5001');
          var target_img = scope.data.target_image.replace('public', 'http://10.0.2.90:5001');
          img.src = expect_img;
          img.onload = function () {

            var width = this.width;
            var height = this.height;
            var imgAreaEl = element.find('.image-area')[0];
            var svg = d3.select(imgAreaEl)
              .append('svg')
              .attr('width', width)
              .attr('height', height);


            var svg_target_img = svg.append("image")
              .attr("class", "target-image")
              .attr("image-rendering", "optimizeQuality")
              .attr("x", '0')
              .attr("y", '0');

            svg_target_img.attr('height', height)
              .attr('width', width)
              .attr('xlink:href', target_img);

            var svg_expect_img = svg.append("image")
              .attr("class", "expect-image")
              .attr("image-rendering", "optimizeQuality")
              .attr("x", '0')
              .attr("y", '0')
              .attr("opacity", scope.opacity);

            svg_expect_img.attr('height', height)
              .attr('width', width)
              .attr('xlink:href', expect_img);

            svg.selectAll('.vector-line')
              .data(scope.data.vector)
              .enter()
              .append('svg:line')
              .attr('class', 'vector-line')
              .attr({
                x1: function (d) {
                  return d.x;
                },
                y1: function (d) {
                  return d.y;
                },
                x2: function (d) {
                  return d.x + d.dx;
                },
                y2: function (d) {
                  return d.y + d.dy;
                },
                stroke: 'red'
              });
          };
        });

      }
    }
  }]);

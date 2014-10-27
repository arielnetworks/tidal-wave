angular.module('app', ['ui.bootstrap'])
  .controller('myController', ['$scope', '$location', function ($scope, $location) {

    $scope.items = [];
    $scope.socket = io('ws://' + $location.host() + ':' + $location.port());
    $scope.socket.on('message', function(msg) {
      $scope.$apply(function(){
        if (msg.status == 'SUSPICIOUS') {
          var item = {
            data : msg,
            mode: "both",
            opacity: 0.5
          };
          $scope.items.push(item);
        }
      });
    });

    $scope.socket.send({
      'expect_path': 'public/test_images/widget_common_normal',
      'target_path': 'public/test_images/widget_common_normal_scratch'
      //'expect_path': 'public/images',
      //'target_path': 'public/images2'
    });

  }]);

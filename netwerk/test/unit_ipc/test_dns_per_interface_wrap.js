//
// Run test script in content process instead of chrome (xpcshell's default)
//

function run_test() {
  run_test_in_child("../unit/test_dns_per_interface.js");
}

<?php
/**
 * @file
 * PHPUnit tests for core Bee functionality.
 */

use PHPUnit\Framework\TestCase;
/**
 * Tests core Bee functionality.
 */
class BeeCoreTest extends TestCase {

  /**
   * Make sure that `bee` is installed correctly.
   */
  public function test_bee_is_installed_correctly() {
    // A '0' exit status means success.
    exec('bee', $output, $result);
    $this->assertEquals(0, $result);
  }

  /**
   * Make sure that unknown commands trigger an error.
   */
  public function test_unknown_command_triggers_an_error() {
    $command = 'spoon';
    $output = shell_exec("bee $command");
    $this->assertStringContainsString("There is no '$command' command.", (string) $output);
  }

  /**
   * Make sure that required arguments are actually required.
   */
  public function test_missing_required_argument_triggers_an_error() {
    // `config-get` has a required argument: 'file'.
    $output = shell_exec('bee config-get');
    $this->assertStringContainsString("Argument 'file' is required.", (string) $output);
  }

  /**
   * Make sure that command aliases work.
   */
  public function test_commands_can_be_called_via_an_alias() {
    // `st` is an alias for `status`.
    $output_command = shell_exec('bee status');
    $output_alias = shell_exec('bee st');
    $this->assertEquals($output_command, $output_alias);
  }

  /**
   * Make sure that the `--root` option works.
   *
   * Test running `bee` from outside the installation without the '--root'
   * option, and then with the '--root' option.
   */
  public function test_root_global_option_works() {
    $output_no_root = shell_exec('cd ../ && bee status');
    $this->assertStringContainsString('No Backdrop installation found.', (string) $output_no_root);

    $output_root = shell_exec('cd ../ && bee --root=backdrop status');
    $this->assertStringNotContainsString('No Backdrop installation found.', (string) $output_root);
  }

  /**
   * Make sure that the `--base-url` option works.
   *
   * Test running the `user-login` command of `bee` without the '--base-url'
   * option, and then with the '--base-url' option.
   */
  public function test_base_url_global_option_works() {
    global $bee_test_backdrop_base_url;
    $output_no_base_url = shell_exec('bee user-login admin');
    $this->assertStringContainsString("Use the following link to login as 'admin':", (string) $output_no_base_url);
    $this->assertStringContainsString('http://backdrop/user/reset/1/', (string) $output_no_base_url);

    $output_base_url = shell_exec("bee --base-url=$bee_test_backdrop_base_url user-login admin");
    $this->assertStringContainsString("Use the following link to login as 'admin':", (string) $output_base_url);
    $this->assertStringContainsString("$bee_test_backdrop_base_url/user/reset/1/", (string) $output_base_url);
  }

  /**
   * Make sure that the `--yes/--y` options work.
   */
  public function test_yes_global_option_works() {
    $file = 'bee.test';
    $option = 'foo';
    $value = 'bar';

    $output_yes = shell_exec("bee config-set --yes $file $option $value");
    $this->assertStringContainsString("The '$file' config file doesn't exist.", (string) $output_yes);
    $this->assertStringContainsString("'$option' was set to '$value' in '$file'.", (string) $output_yes);

    $file2 = $file . '2';

    $output_y = shell_exec("bee config-set --y $file2 $option $value");
    $this->assertStringContainsString("The '$file2' config file doesn't exist.", (string) $output_y);
    $this->assertStringContainsString("'$option' was set to '$value' in '$file2'.", (string) $output_y);

    // Cleanup config files.
    // Using `find` allows us to find and delete the necessary files without
    // knowing the name of the config directory.
    exec("find files -type f -iname '$file.json' -delete");
    exec("find files -type f -iname '$file2.json' -delete");
  }

  /**
   * Make sure that the `--debug/--d` options work.
   */
  public function test_debug_global_option_works() {
    $output_debug = shell_exec("bee status --debug");
    $this->assertStringContainsString("'Debug' mode enabled.", (string) $output_debug);

    $output_d = shell_exec("bee status --d");
    $this->assertStringContainsString("'Debug' mode enabled.", (string) $output_d);
  }

}

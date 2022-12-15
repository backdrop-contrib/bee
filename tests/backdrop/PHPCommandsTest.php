<?php
/**
 * @file
 * PHPUnit tests for Bee PHP commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's PHP command.
 */
class PHPCommandsTest extends TestCase {

  /**
   * Make sure that the eval command works.
   */
  public function test_eval_command_works() {
    // Test a simple command.
    $output_simple = shell_exec('bee eval \'echo "foo" . "bar";\'');
    $this->assertStringContainsString('foobar', (string) $output_simple);

    // Test that the terminal semicolon is optional.
    $output_semicolon = shell_exec('bee eval \'echo "Hello world."\'');
    $this->assertStringContainsString('Hello world.', (string) $output_semicolon);

    // Test that calling a Backdrop function works.
    exec('bee eval \'config_set("test.settings", "value", "foobar")\'');
    $output_backdrop = shell_exec('bee eval \'echo config_get("test.settings", "value")\'');
    $this->assertStringContainsString('foobar', (string) $output_backdrop);

    // Cleanup test config file.
    exec('bee eval \'config("test.settings")->delete();\'');
  }

  /**
   * Make sure that the php-script command works.
   */
  public function test_script_command_works() {
    // Create a simple script.
    exec('echo "<?php echo \"foo\" . \"bar\";" > /app/backdrop/simple.php');

    $this->assertTrue(file_exists('/app/backdrop/simple.php'));

    // Test simple command.
    $output_simple = shell_exec('bee scr simple.php');
    $this->assertStringContainsString('foobar', (string) $output_simple);

    // Remove for future tests.
    exec('rm /app/backdrop/simple.php');

    // Create a script that calls a Backdrop function.
    exec('echo "<?php config_set(\"test.settings\", \"value\", \"foobar\");" > /app/backdrop/config-set.php');

    $this->assertTrue(file_exists('/app/backdrop/config-set.php'));

    exec('echo "<?php echo config_get(\"test.settings\", \"value\");" > /app/backdrop/config-get.php');

    $this->assertTrue(file_exists('/app/backdrop/config-get.php'));

    // Test that calling a Backdrop function works.
    exec('bee php-script /app/backdrop/config-set.php');
    $output_backdrop = shell_exec('bee php-script config-get.php');
    $this->assertStringContainsString('foobar', (string) $output_backdrop);

    // Remove for future tests.
    exec('rm /app/backdrop/config-set.php');
    exec('rm /app/backdrop/config-get.php');

    // Cleanup test config file.
    exec('bee eval \'config("test.settings")->delete();\'');
  }

}

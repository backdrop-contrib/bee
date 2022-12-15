<?php
/**
 * @file
 * PHPUnit tests for Bee PHP commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's PHP command.
 */
class ScriptCommandsTest extends TestCase {

  /**
   * Make sure that the eval command works.
   */
  public function test_eval_command_works() {
    // Create a simple script.
    exec('"<?php echo \"foo\" . \"bar\";" > /app/backdrop/simple.php');

    $this->assertTrue(file_exists('/app/backdrop/simple.php'));

    // Test simple command.
    $output_simple = shell_exec('bee scr simple.php');
    $this->assertStringContainsString('foobar', (string) $output_simple);

    // Remove for future tests.
    exec('rm /app/backdrop/simple.php');

    // Create a script that calls a Backdrop function.
    exec('"<?php config_set(\"test.settings\", \"value\", \"foobar\");" > /app/backdrop/backdrop-function.php');

    $this->assertTrue(file_exists('/app/backdrop/backdrop-function.php'));

    // Test that calling a Backdrop function works.
    exec('bee php-script /app/backdrop/backdrop-function.php');
    $output_backdrop = shell_exec('bee eval \'echo config_get("test.settings", "value")\'');
    $this->assertStringContainsString('foobar', (string) $output_backdrop);

    // Remove for future tests.
    exec('rm /app/backdrop/backdrop-function.php');

    // Cleanup test config file.
    exec('bee eval \'config("test.settings")->delete();\'');
  }

}

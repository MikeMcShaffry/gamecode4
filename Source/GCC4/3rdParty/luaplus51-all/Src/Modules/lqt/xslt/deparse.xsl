<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="text" />
<xsl:strip-space elements="*"/>
<xsl:template match="/lib">
    <xsl:text>#include &lt;Qt>&#xA;</xsl:text>
    <xsl:apply-templates select="function|type"/>
</xsl:template>

<xsl:template match="type">
    <xsl:if test="struct">
        <xsl:text>class </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:if test="struct/*">
            <xsl:if test="*/base">
                <xsl:text> : </xsl:text>
                <xsl:for-each select="*/base">
                    <xsl:if test='@virtual'>
                        <xsl:text>virtual </xsl:text>
                    </xsl:if>
                    <xsl:value-of select="@access"/>
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="@class"/>
                    <xsl:if test="position() &lt; last()">
                        <xsl:text>, </xsl:text>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
            <xsl:text> {&#xa;</xsl:text>
            <xsl:for-each select="struct/method|struct/function">
                <xsl:if test="position() = 1 or not(@access = preceding-sibling::*[1]/@access)">
                    <xsl:value-of select="@access"/>
                    <xsl:text>:&#xa;</xsl:text>
                </xsl:if>
                <xsl:text>&#x9;</xsl:text>
                <xsl:if test="virtual">
                    <xsl:text>virtual </xsl:text>
                </xsl:if>
                <xsl:if test="name() = 'function'">
                    <xsl:text>static </xsl:text>
                </xsl:if>

    <xsl:variable name="args">
        <xsl:for-each select="param">
            <xsl:call-template name="declare-var">
                <xsl:with-param name="type" select="*"/>
                <xsl:with-param name="var" select="@name"/>
            </xsl:call-template>
            <xsl:if test="position() &lt; last()"><xsl:text>, </xsl:text></xsl:if>
        </xsl:for-each>
    </xsl:variable>

                <xsl:variable name="method">
                    <xsl:call-template name="substring-after-last">
                        <xsl:with-param name="text" select="@name"/>
                        <xsl:with-param name="chars">::</xsl:with-param>
                    </xsl:call-template>
                    <xsl:value-of select="concat('(', $args, ')')"/>
                </xsl:variable>

                <xsl:call-template name="declare-var">
                    <xsl:with-param name="type" select="return/*"/>
                    <xsl:with-param name="var" select="$method"/>
                </xsl:call-template>

                <xsl:if test="this//@qualifier">
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="this//@qualifier"/>
                </xsl:if>
                <!--<xsl:value-of select="@name"/>-->
                <xsl:text>;&#xa;</xsl:text>
            </xsl:for-each>
            <xsl:text>}</xsl:text>
        </xsl:if>
        <xsl:text>;&#xA;&#xa;</xsl:text>
    </xsl:if>
</xsl:template>

<xsl:template match="function">
    <xsl:variable name="args">
        <xsl:for-each select="param">
            <xsl:call-template name="declare-var">
                <xsl:with-param name="type" select="*"/>
                <xsl:with-param name="var" select="@name"/>
            </xsl:call-template>
            <xsl:if test="position() &lt; last()"><xsl:text>, </xsl:text></xsl:if>
        </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="f">
        <xsl:call-template name="declare-var">
            <xsl:with-param name="var">
                <xsl:value-of select="@name"/>
                <xsl:value-of select="concat('(', $args, ')')"/>
            </xsl:with-param>
            <xsl:with-param name="type" select="return"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:value-of select="$f"/>
    <xsl:text>;&#xA;</xsl:text>
</xsl:template>

<xsl:template name="declare-var">
    <xsl:param name="var"/>
    <xsl:param name="type"/>
    <xsl:variable name="varname">
        <xsl:if test="$var">
            <xsl:value-of select="concat(' ', $var)"/>
        </xsl:if>
    </xsl:variable>
    <xsl:variable name="v">
        <xsl:apply-templates select="$type" mode="typedef">
            <xsl:with-param name="var"><xsl:text>#</xsl:text></xsl:with-param>
        </xsl:apply-templates>
    </xsl:variable>
    <xsl:value-of select='concat(substring-before($v, "#"), $varname, substring-after($v, "#"))'/>
</xsl:template>


<xsl:template match="pointer|reference" mode="typedef">
    <xsl:param name="var"/>
    <xsl:apply-templates select="type|function|pointer|reference" mode="typedef">
        <xsl:with-param name="var">
            <xsl:value-of select="substring-before($var, '#')"/>
            <xsl:choose>
                <xsl:when test="name() = 'pointer'"><xsl:text>*</xsl:text></xsl:when>
                <xsl:otherwise><xsl:text>&amp;</xsl:text></xsl:otherwise>
            </xsl:choose>
            <xsl:if test="@qualifier">
                <xsl:text> </xsl:text>
                <xsl:value-of select="@qualifier"/>
            </xsl:if>
            <xsl:text>#</xsl:text>
            <xsl:value-of select="substring-after($var, '#')"/>
        </xsl:with-param>
    </xsl:apply-templates>
</xsl:template>

<!-- ugly cut and paste... refactor -->
<xsl:template match="function" mode="typedef">
    <xsl:param name="var"/>
    <xsl:variable name="args">
        <xsl:for-each select="param">
            <xsl:call-template name="declare-var">
                <xsl:with-param name="type" select="*"/>
                <xsl:with-param name="var" select="@name"/>
            </xsl:call-template>
            <xsl:if test="position() &lt; last()"><xsl:text>, </xsl:text></xsl:if>
        </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="f">
        <xsl:call-template name="declare-var">
            <xsl:with-param name="var">
                <xsl:value-of select="concat('(', $var, ')')"/>
                <xsl:value-of select="concat('(', $args, ')')"/>
            </xsl:with-param>
            <xsl:with-param name="type" select="return"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:value-of select="$f"/>
</xsl:template>

<xsl:template match="type" mode="typedef">
    <xsl:param name="var"/>
    <xsl:if test="@qualifier">
        <xsl:value-of select="@qualifier"/>
        <xsl:text> </xsl:text>
    </xsl:if>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="$var"/>
</xsl:template>

<!-- from xsltsl -->
<xsl:template name="substring-after-last">

    <xsl:param name="text"/>
    <xsl:param name="chars"/>

    <xsl:choose>

      <xsl:when test="contains($text, $chars)">
        <xsl:variable name="last" select="substring-after($text, $chars)"/>

	<xsl:choose>
	  <xsl:when test="contains($last, $chars)">

	    <xsl:call-template name="substring-after-last">
	      <xsl:with-param name="text" select="$last"/>
	      <xsl:with-param name="chars" select="$chars"/>
	    </xsl:call-template>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:value-of select="$last"/>
	  </xsl:otherwise>
	</xsl:choose>

      </xsl:when>

      <xsl:otherwise>
        <xsl:value-of select="$text"/>
      </xsl:otherwise>

    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>

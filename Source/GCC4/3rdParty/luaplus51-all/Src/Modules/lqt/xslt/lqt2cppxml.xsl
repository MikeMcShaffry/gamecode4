<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
        xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" indent="yes"/>
<xsl:strip-space elements="*"/>

<xsl:template match="/">
<lib>
  <xsl:apply-templates/>
</lib>
</xsl:template>

<xsl:template match="Class">
  <type name="{@fullname}">
    <struct>
      <xsl:call-template name="base-classes">
        <xsl:with-param name="bases" select="@bases"/>
      </xsl:call-template>
      <xsl:apply-templates match="Function|FunctionDefinition" mode="member"/>
    </struct>
  </type>
</xsl:template>

<xsl:template match="Function|FunctionDefinition" mode="member">
  <xsl:variable name="type">
    <xsl:choose>
      <xsl:when test="@name = ../@name">
        <xsl:value-of select="'constructor'"/>
      </xsl:when>
      <xsl:when test="@name = concat('~', ../@name)">
        <xsl:value-of select="'destructor'"/>
      </xsl:when>
      <xsl:when test="@static">
        <xsl:value-of select="'function'"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="'method'"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:element name="{$type}">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
    <xsl:attribute name="access"><xsl:value-of select="@access"/></xsl:attribute>
    <return>
      <xsl:call-template name="convert-type">
        <xsl:with-param name="reference" select="@type_reference"/>
        <xsl:with-param name="indirections" select="@indirections"/>
        <xsl:with-param name="type" select="@type_base"/>
        <xsl:with-param name="qualifier">
          <xsl:call-template name="make-qualifier">
            <xsl:with-param name="const" select="@type_constant"/>
            <xsl:with-param name="volatile" select="@type_volatile"/>
          </xsl:call-template>
        </xsl:with-param>
      </xsl:call-template>
    </return>
    <xsl:if test="not(@static)">
      <this>
        <xsl:call-template name="convert-type">
          <xsl:with-param name="indirections" select="1"/>
          <xsl:with-param name="type" select="@member_of_class"/>
          <xsl:with-param name="qualifier">
            <xsl:call-template name="make-qualifier">
              <xsl:with-param name="const" select="@constant"/>
              <xsl:with-param name="volatile" select="@volatile"/>
            </xsl:call-template>
          </xsl:with-param>
        </xsl:call-template>
      </this>
    </xsl:if>
    <xsl:apply-templates match="Argument" mode="param"/>
    <xsl:choose>
      <xsl:when test="@abstract"><abstract/></xsl:when>
      <xsl:when test="@virtual"><virtual/></xsl:when>
    </xsl:choose>
  </xsl:element>
</xsl:template>

<xsl:template match="Argument" mode="param">
  <param>
    <xsl:if test="@name != ''">
      <xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
    </xsl:if>
    <xsl:if test="@default='1'">
      <default value="{@defaultvalue}"/>
    </xsl:if>
    <xsl:call-template name="convert-type">
      <xsl:with-param name="reference" select="@type_reference"/>
      <xsl:with-param name="indirections" select="@indirections"/>
      <xsl:with-param name="type" select="@type_base"/>
      <xsl:with-param name="qualifier">
        <xsl:call-template name="make-qualifier">
          <xsl:with-param name="const" select="@type_constant"/>
          <xsl:with-param name="volatile" select="@type_volatile"/>
        </xsl:call-template>
      </xsl:with-param>
    </xsl:call-template>
  </param>
</xsl:template>

<xsl:template name="make-qualifier">
  <xsl:param name="const"/>
  <xsl:param name="volatile"/>
  <xsl:choose>
    <xsl:when test="$const and $volatile"><xsl:value-of select="'const volatile'"/></xsl:when>
    <xsl:when test="$const"><xsl:value-of select="'const'"/></xsl:when>
    <xsl:when test="$volatile"><xsl:value-of select="'volatile'"/></xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template name="convert-type">
  <xsl:param name="reference"/>
  <xsl:param name="indirections"/>
  <xsl:param name="type"/>
  <xsl:param name="qualifier"/>

  <xsl:choose>
    <xsl:when test="$reference">
      <reference>
        <xsl:call-template name="convert-type">
          <xsl:with-param name="indirections" select="$indirections"/>
          <xsl:with-param name="type" select="$type"/>
          <xsl:with-param name="qualifier" select="$qualifier"/>
        </xsl:call-template>
      </reference>
    </xsl:when>
    <xsl:when test="$indirections and $indirections &gt; 0">
      <pointer>
        <xsl:call-template name="convert-type">
          <xsl:with-param name="indirections" select="$indirections - 1"/>
          <xsl:with-param name="type" select="$type"/>
          <xsl:with-param name="qualifier" select="$qualifier"/>
        </xsl:call-template>
      </pointer>
    </xsl:when>
    <xsl:otherwise>
      <type name="{$type}">
        <xsl:if test="$qualifier != ''">
          <xsl:attribute name="qualifier"><xsl:value-of select="$qualifier"/></xsl:attribute>
        </xsl:if>
      </type>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="base-classes">
  <xsl:param name="bases"/>
  <xsl:if test="contains($bases, ';')">
    <base class="{substring-before($bases, ';')}" access="public"/>
    <xsl:call-template name="base-classes">
      <xsl:with-param name="bases" select="substring-after($bases, ';')"/>
    </xsl:call-template>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>
